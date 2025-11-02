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
#define GPIO_OFFSET 512
#define DATA_GPIO 229
#define CLK_GPIO 230
#define SYNC_GPIO 228
#define BAUD_RATE       9600
// Tempo de atraso de 1 bit (em nanosegundos), adaptado para o kernel
#define BIT_TIME_NS     (1000000000L / BAUD_RATE)
#define BIT_TIME_US     (1000000UL / BAUD_RATE)

static struct gpio_desc *data_gpiod;
static struct gpio_desc *clk_gpiod;
static struct gpio_desc *sync_gpiod;
static int data_irq_num;

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
static uint16_t input_data = 0x0000;
static uint16_t previous_data = 0x0000;


static int lastSync = 1;

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


// Manipulador de Interrupção para o Start Bit (Borda de Descida)
static irqreturn_t data_interrupt_handler(int irq, void *dev_id)
{
    // A interrupção é rápida; agendamos o trabalho real para um thread
    // ou usamos um workqueue para evitar bloquear outros IRQs.
    // Para simplificar, neste exemplo, forçamos a leitura aqui:
    static int bit_counter = 0;
    static uint16_t received_data = 0;
    // 1. O dado é lido quando o CLK está em um estado (ex: subida)
    int bit_val = gpiod_get_value(data_gpiod);

    if (bit_val == 1) {
    	received_data |= 1 << bit_counter; // MSB primeiro
    }
    // 2. Incrementa o contador
    bit_counter++;

    if (bit_counter == 16) {
	    // Se 16 bits foram lidos, a leitura está completa
	    // Idealmente, você notificaria um thread do espaço do usuário aqui.
	    bit_counter = 0;
	    pr_info("%s: Dados Síncronos Recebidos: %d = 0x%04X\n", DRV_NAME, received_data, received_data);
	    input_data = received_data;
	    processData();
	    received_data = 0;
	    //print_binary(received_data);
    }
    lastSync = !lastSync;
    gpiod_set_value(sync_gpiod, lastSync);
    return IRQ_HANDLED;
}

// === Funções de Inicialização e Limpeza do Módulo ===

static int gpio_reader_init(void)
{
    int ret;
    
    // 1. Alocar e configurar o pino GPIO (gpiod)
    //data_gpiod = gpiod_get_from_platform_data(NULL, "data_line", 0);
    data_gpiod = gpio_to_desc(DATA_GPIO);
    clk_gpiod = gpio_to_desc(CLK_GPIO);
    sync_gpiod = gpio_to_desc(SYNC_GPIO);
    if (data_gpiod == NULL || clk_gpiod == NULL || sync_gpiod == NULL) {
        pr_err("%s: Falha ao obter GPIO RX\n", DRV_NAME);
        goto err_direction_rx;
    }
    
    // Configurar pino de dados como INPUT
    ret = gpiod_direction_input(data_gpiod);
    if (ret < 0) { goto err_direction_rx; }

    ret = gpiod_direction_output(sync_gpiod,0);
    if (ret < 0) { goto err_direction_rx; }

    // 2. Obter o número IRQ
    data_irq_num = gpiod_to_irq(clk_gpiod);
    if (data_irq_num < 0) {
        pr_err("%s: Falha ao mapear GPIO para IRQ\n", DRV_NAME);
        ret = data_irq_num;
        goto err_direction_rx;
    }

    // // 3. Registrar o manipulador de IRQ para bordas de subida (Start Bit)
    ret = request_irq(data_irq_num, data_interrupt_handler, 
                      IRQF_TRIGGER_RISING | IRQF_SHARED, // Subida (Start Bit) + Compartilhável
                      DRV_NAME, (void *)DRV_NAME);

    if (ret) {
        pr_err("%s: Falha ao registrar IRQ %d\n", DRV_NAME, data_irq_num);
        goto err_direction_rx;
    }

    gpiod_set_value(sync_gpiod, lastSync);
    
    pr_info("%s: Módulo carregado. Esperando Start Bit (IRQ %d)\n", DRV_NAME, data_irq_num);
    return 0;

err_direction_rx:
    gpiod_put(data_gpiod);
    gpiod_put(clk_gpiod);
    gpiod_put(sync_gpiod);
    return ret;
}

static void gpio_reader_exit(void)
{
    free_irq(data_irq_num, (void *)DRV_NAME);
    gpiod_put(data_gpiod);
    gpiod_put(clk_gpiod);
    pr_info("%s: Módulo descarregado.\n", DRV_NAME);
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
    error = gpio_reader_init();
    if (error) {
        printk(KERN_ERR "joy_driver_module: Failed to register gpio\n");
        input_free_device(joy_input_dev);
        return error;
    }
    printk(KERN_INFO "joy_driver_module: Module loaded\n");
    return 0;
}

static void __exit joy_driver_exit(void) {
    input_unregister_device(joy_input_dev);
    gpio_reader_exit();
    printk(KERN_INFO "joy_driver_module: Module unloaded\n");
}

module_init(joy_driver_init);
module_exit(joy_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Virtual Gamepad Kernel Module");
