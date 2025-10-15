// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/types.h>

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

static void readGpio(void) {
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
        readGpio();
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
    printk(KERN_INFO "joy_driver_module: Module loaded\n");
    return 0;
}

static void __exit joy_driver_exit(void) {
    // del_timer_sync(&joy_timer);
    timer_delete_sync(&joy_timer);
    input_unregister_device(joy_input_dev);
    printk(KERN_INFO "joy_driver_module: Module unloaded\n");
}

module_init(joy_driver_init);
module_exit(joy_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Virtual Gamepad Kernel Module");
