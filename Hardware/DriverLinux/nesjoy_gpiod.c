// SPDX-License-Identifier: GPL-2.0
// nesjoy_gpiod.c - Driver de joystick via protocolo NES estendido (LATCH/CLOCK/DATA) usando GPIO.
// Este driver atua como o console, emitindo LATCH/CLOCK e lendo DATA do "controle" (ESP32).
// Protocolo estendido: 11 bits (A, B, Select, Start, Up, Down, Left, Right, C, D, Push)

#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define DRV_NAME "nesjoy_gpiod"

// Número de bits no protocolo (11)
#define NES_BITS 11

// Ordem (bit0→bit10): A, B, Select, Start, Up, Down, Left, Right, C, D, Push
static const unsigned int nes_keycodes[NES_BITS] = {
    BTN_A, BTN_B, BTN_SELECT, BTN_START,
    BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT,
    BTN_X, BTN_Y,             // C, D
    BTN_THUMBL                // Push-button
};

static int poll_interval_ms = 2; // ~500 Hz
module_param(poll_interval_ms, int, 0644);
MODULE_PARM_DESC(poll_interval_ms, "Intervalo de sondagem em ms (default 2ms)");

struct nesjoy {
    struct device *dev;
    struct input_dev *input;
    struct gpio_desc *gpio_latch; // out
    struct gpio_desc *gpio_clk;   // out
    struct gpio_desc *gpio_data;  // in
    struct task_struct *thread;
    bool running;
    u16 last_state;
};

// Lê NES_BITS (11) bits, ativo-em-0 (0 = pressionado no fio DATA)
static u16 nesjoy_read_bits(struct nesjoy *nj)
{
    int i;
    u16 bits = 0;

    // Pulso de LATCH: alto para carregar o shift register no "controle"
    gpiod_set_value_cansleep(nj->gpio_latch, 1);
    udelay(12);
    gpiod_set_value_cansleep(nj->gpio_latch, 0);
    udelay(6);

    // Primeiro bit já disponível, depois avançar com clock
    for (i = 0; i < NES_BITS; i++) {
        int v = gpiod_get_value_cansleep(nj->gpio_data);
        if (v < 0) v = 1; // em caso de erro, trata como solto
        // 0 = pressed no fio, armazenamos pressed = 1
        bits |= ((v == 0) ? 1 : 0) << i;

        // Pulso de clock para próximo bit
        gpiod_set_value_cansleep(nj->gpio_clk, 1);
        udelay(6);
        gpiod_set_value_cansleep(nj->gpio_clk, 0);
        udelay(6);
    }
    return bits;
}

static int nesjoy_thread_fn(void *data)
{
    struct nesjoy *nj = data;

    while (!kthread_should_stop()) {
        u16 state = nesjoy_read_bits(nj);

        // Reportar botões
        for (int i = 0; i < NES_BITS; i++) {
            int pressed = (state >> i) & 0x1;
            input_report_key(nj->input, nes_keycodes[i], pressed);
        }
        input_sync(nj->input);

        if (poll_interval_ms < 1) poll_interval_ms = 1;
        msleep(poll_interval_ms);
    }
    return 0;
}

static int nesjoy_probe(struct platform_device *pdev)
{
    struct nesjoy *nj;
    struct input_dev *input;
    int err;

    nj = devm_kzalloc(&pdev->dev, sizeof(*nj), GFP_KERNEL);
    if (!nj) return -ENOMEM;

    nj->dev = &pdev->dev;
    nj->gpio_latch = devm_gpiod_get(&pdev->dev, "latch", GPIOD_OUT_LOW);
    if (IS_ERR(nj->gpio_latch))
        return dev_err_probe(&pdev->dev, PTR_ERR(nj->gpio_latch), "gpio latch\n");

    nj->gpio_clk = devm_gpiod_get(&pdev->dev, "clk", GPIOD_OUT_LOW);
    if (IS_ERR(nj->gpio_clk))
        return dev_err_probe(&pdev->dev, PTR_ERR(nj->gpio_clk), "gpio clk\n");

    nj->gpio_data = devm_gpiod_get(&pdev->dev, "data", GPIOD_IN);
    if (IS_ERR(nj->gpio_data))
        return dev_err_probe(&pdev->dev, PTR_ERR(nj->gpio_data), "gpio data\n");

    input = devm_input_allocate_device(&pdev->dev);
    if (!input) return -ENOMEM;

    input->name = "NES Joystick (GPIO)";
    input->phys = "nesjoy/input0";
    input->id.bustype = BUS_HOST;
    input->id.vendor  = 0x0001;
    input->id.product = 0x00NES;
    input->id.version = 0x0001;

    __set_bit(EV_KEY, input->evbit);
    for (int i = 0; i < NES_BITS; i++)
        __set_bit(nes_keycodes[i], input->keybit);

    err = input_register_device(input);
    if (err) return err;

    nj->input = input;
    platform_set_drvdata(pdev, nj);

    nj->thread = kthread_run(nesjoy_thread_fn, nj, DRV_NAME "_thr");
    if (IS_ERR(nj->thread)) {
        err = PTR_ERR(nj->thread);
        nj->thread = NULL;
        return err;
    }

    dev_info(&pdev->dev, "nesjoy_gpiod iniciado (11 bits)\n");
    return 0;
}

static int nesjoy_remove(struct platform_device *pdev)
{
    struct nesjoy *nj = platform_get_drvdata(pdev);
    if (nj->thread) kthread_stop(nj->thread);
    return 0;
}

static const struct of_device_id nesjoy_of_match[] = {
    { .compatible = "example,nesjoy-gpiod" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, nesjoy_of_match);

static struct platform_driver nesjoy_driver = {
    .probe  = nesjoy_probe,
    .remove = nesjoy_remove,
    .driver = {
        .name = DRV_NAME,
        .of_match_table = nesjoy_of_match,
    },
};

module_platform_driver(nesjoy_driver);

MODULE_AUTHOR("Você");
MODULE_DESCRIPTION("Driver NES joystick via GPIO (11 bits com Push)");
MODULE_LICENSE("GPL");
