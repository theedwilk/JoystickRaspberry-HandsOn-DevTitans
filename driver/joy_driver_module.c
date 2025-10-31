// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/time.h>        // Para tempos de alta resolução
#include <linux/jiffies.h>     // Para temporização baseada em ticks do kernel


// === Definições ===
#define DRV_NAME "joy_driver_module"
#define RX_GPIO_NUM     15 // Exemplo de pino RPi GPIO (BCM)
#define GPIO_OFFSET 512
#define DATA_GPIO RX_GPIO_NUM + GPIO_OFFSET
#define BAUD_RATE       9600
// Tempo de atraso de 1 bit (em nanosegundos), adaptado para o kernel
#define BIT_TIME_NS     (1000000000L / BAUD_RATE)
#define BIT_TIME_US     (1000000UL / BAUD_RATE)

static struct gpio_desc *rx_gpiod;
static int rx_irq_num;

/* Prefer udelay() inside modules to avoid referencing architecture-specific
 * helper symbols like __bad_ndelay which are not exported on some kernels.
 * Use microsecond resolution (rounded) for bit-banging delays. */
#define delay_bit() udelay(BIT_TIME_US)
#define delay_half_bit() udelay(BIT_TIME_US / 2)

#define LOOP_INTERVAL_MS 10
#define B_BUTTON_MASK 1
#define A_BUTTON_MASK 1 << 1
#define Y_BUTTON_MASK 1 << 2
#define X_BUTTON_MASK 1 << 3
#define SEL_BUTTON_MASK 1 << 4
#define ST_BUTTON_MASK 1 << 5
#define DOWN_BUTTON_MASK 1 << 6
#define RIGHT_BUTTON_MASK 1 << 7
#define UP_BUTTON_MASK 1 << 8
#define LEFT_BUTTON_MASK 1 << 9

struct button_map {
    uint16_t bit;
    int code;
};

static struct button_map buttons[] = {
    {A_BUTTON_MASK, BTN_SOUTH},
    {B_BUTTON_MASK, BTN_EAST},
    {X_BUTTON_MASK, BTN_WEST},
    {Y_BUTTON_MASK, BTN_NORTH},
    {SEL_BUTTON_MASK, BTN_SELECT},
    {ST_BUTTON_MASK, BTN_START},
    {UP_BUTTON_MASK, BTN_DPAD_UP},
    {DOWN_BUTTON_MASK, BTN_DPAD_DOWN},
    {LEFT_BUTTON_MASK, BTN_DPAD_LEFT},
    {RIGHT_BUTTON_MASK, BTN_DPAD_RIGHT},
};

typedef enum {
    DATA_COLLECT = 1,
    DATA_OUTPUT
} State_t;

static struct input_dev *joy_input_dev;
static struct timer_list joy_timer;
static uint16_t input_data = 0x0000;
static uint16_t previous_data = 0x0000;

static void print_binary(uint16_t value) {
    char buf[16];
    int i;
    for (i = 9; i >= 0; i--) {
        buf[9 - i] = (value & (1 << i)) ? '1' : '0';
    }
    buf[10] = '\0';
    printk(KERN_INFO "joy_driver_module: input_data = %s\n", buf);
}

/**
 * Função principal para ler 2 bytes assíncronos (UART Bit-Banging).
 * NOTA: Esta função DEVE ser chamada de um contexto de Thread ou Workqueue
 * (não diretamente do IRQ Handler) pois ela usa atrasos longos.
 */
static uint16_t read_2_bytes_uart_module(void)
{
    uint16_t received_data = 0;

    // 1. Esperar a Borda de Descida do Start Bit (já assumido pelo IRQ)
    // A função é chamada após a borda de descida.

    // 2. Sincronização: Esperar o Meio Bit para ler no centro do primeiro bit de dados
    delay_half_bit();

    // 3. Loop de leitura (16 bits)
    for (int i = 0; i < 16; i++) { // LSB primeiro (padrão UART)
        int bit_val = gpiod_get_value(rx_gpiod);
        
        if (bit_val == 1) {
            received_data |= (1 << i);
        }
        
        // Esperar o tempo completo de um bit para ler o próximo
        delay_bit();
    }
    
    // O Stop Bit (HIGH) será ignorado, assumindo que o estado de repouso é HIGH.

    pr_info("%s: Dados Assíncronos Lidos: 0x%04X\n", DRV_NAME, received_data);
    print_binary(received_data);
    input_data = received_data;
    return received_data;
}

// Manipulador de Interrupção para o Start Bit (Borda de Descida)
static irqreturn_t rx_interrupt_handler(int irq, void *dev_id)
{
    // A interrupção detectou o Start Bit (transição HIGH -> LOW).
    // O trabalho pesado (leitura bit a bit) deve ser delegado para evitar latência no IRQ.
    
    // Idealmente, você usaria uma Workqueue ou Thread Kernel, mas
    // para um exemplo simples, chamamos a função de leitura que usa ndelay:
    read_2_bytes_uart_module();
    
    // Após a leitura, o IRQ deve ser rearmado (o que já acontece por padrão
    // se o pino não for reconfigurado, mas pode ser necessário desabilitar
    // a interrupção temporariamente dependendo do design do hardware).
    
    return IRQ_HANDLED;
}

// === Funções de Inicialização e Limpeza do Módulo ===

static int uart_reader_init(void)
{
    int ret;
    
    // 1. Alocar e configurar o pino GPIO (gpiod)
    //rx_gpiod = gpiod_get_from_platform_data(NULL, "rx_line", 0);
    rx_gpiod = gpio_to_desc(DATA_GPIO);
    if (rx_gpiod == NULL) {
        pr_err("%s: Falha ao obter GPIO RX\n", DRV_NAME);
        return PTR_ERR(rx_gpiod);
    }
    
    // Configurar pino de dados como INPUT
    ret = gpiod_direction_input(rx_gpiod);
    if (ret < 0) { goto err_direction_rx; }

    // 2. Obter o número IRQ
    rx_irq_num = gpiod_to_irq(rx_gpiod);
    if (rx_irq_num < 0) {
        pr_err("%s: Falha ao mapear GPIO para IRQ\n", DRV_NAME);
        ret = rx_irq_num;
        goto err_direction_rx;
    }

    // // 3. Registrar o manipulador de IRQ para bordas de descida (Start Bit)
    ret = request_irq(rx_irq_num, rx_interrupt_handler, 
                      IRQF_TRIGGER_FALLING | IRQF_SHARED, // Descida (Start Bit) + Compartilhável
                      DRV_NAME, (void *)DRV_NAME);

    if (ret) {
        pr_err("%s: Falha ao registrar IRQ %d\n", DRV_NAME, rx_irq_num);
        goto err_direction_rx;
    }
    
    pr_info("%s: Módulo carregado. Esperando Start Bit (IRQ %d)\n", DRV_NAME, rx_irq_num);
    return 0;

err_direction_rx:
    gpiod_put(rx_gpiod);
    return ret;
}

static void uart_reader_exit(void)
{
    free_irq(rx_irq_num, (void *)DRV_NAME);
    gpiod_put(rx_gpiod);
    pr_info("%s: Módulo descarregado.\n", DRV_NAME);
}

static void readGpio(void) {
    input_data = read_2_bytes_uart_module();
    return;
    static int currInput = 0;
    uint16_t testInputList[] = {A_BUTTON_MASK,B_BUTTON_MASK,X_BUTTON_MASK,Y_BUTTON_MASK, UP_BUTTON_MASK,DOWN_BUTTON_MASK,LEFT_BUTTON_MASK,RIGHT_BUTTON_MASK,0x0000};
    if (testInputList[currInput] == 0x0000) {
        input_data = 0x0000;
        currInput = 0;
    } else {
        input_data |= testInputList[currInput];
        currInput++;
    }
}

static void processData(void) {
    unsigned short changed_bits = input_data ^ previous_data;
    size_t i;
    for (i = 0; i < ARRAY_SIZE(buttons); i++) {
        unsigned short mask = buttons[i].bit;
        if (changed_bits & mask) {
            int button_is_pressed = (input_data & mask) != 0;
            input_report_key(joy_input_dev, buttons[i].code, button_is_pressed);
        }
    }
    input_sync(joy_input_dev);
    previous_data = input_data;
}

static void joy_timer_func(struct timer_list *t) {
    static State_t state = DATA_COLLECT;
    if (state == DATA_COLLECT) {
        //readGpio();
        state = DATA_OUTPUT;
    } else {
        processData();
        print_binary(input_data);
        state = DATA_COLLECT;
    }
    mod_timer(&joy_timer, jiffies + msecs_to_jiffies(LOOP_INTERVAL_MS));
}

static int __init joy_driver_init(void) {
    int i, error;
    joy_input_dev = input_allocate_device();
    if (!joy_input_dev) {
        printk(KERN_ERR "joy_driver_module: Failed to allocate input device\n");
        return -ENOMEM;
    }
    joy_input_dev->name = "Simple Virtual Gamepad";
    joy_input_dev->phys = "joy_driver/input0";
    joy_input_dev->id.bustype = BUS_VIRTUAL;
    joy_input_dev->id.vendor  = 0x1234;
    joy_input_dev->id.product = 0x5678;
    joy_input_dev->id.version = 1;
    joy_input_dev->evbit[0] = BIT_MASK(EV_KEY);
    joy_input_dev->evbit[0] |= BIT_MASK(EV_SYN);
    for (i = 0; i < ARRAY_SIZE(buttons); i++) {
        set_bit(buttons[i].code, joy_input_dev->keybit);
    }
    error = input_register_device(joy_input_dev);
    if (error) {
        printk(KERN_ERR "joy_driver_module: Failed to register input device\n");
        input_free_device(joy_input_dev);
        return error;
    }
    timer_setup(&joy_timer, joy_timer_func, 0);
    mod_timer(&joy_timer, jiffies + msecs_to_jiffies(LOOP_INTERVAL_MS));
    uart_reader_init();
    printk(KERN_INFO "joy_driver_module: Module loaded\n");
    return 0;
}

static void __exit joy_driver_exit(void) {
    // del_timer_sync(&joy_timer);
    timer_delete_sync(&joy_timer);
    input_unregister_device(joy_input_dev);
    uart_reader_exit();
    printk(KERN_INFO "joy_driver_module: Module unloaded\n");
}

module_init(joy_driver_init);
module_exit(joy_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Virtual Gamepad Kernel Module");
