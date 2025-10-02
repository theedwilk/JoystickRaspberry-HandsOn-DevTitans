#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define LOOP_INTERVAL 10000
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
// Standard Linux evdev button codes
#define EVDEV_BUTTON_A  BTN_SOUTH  // Typically 'A' or 'Cross'
#define EVDEV_BUTTON_B  BTN_EAST   // Typically 'B' or 'Circle'
#define EVDEV_BUTTON_X  BTN_WEST   // Typically 'X' or 'Square'
#define EVDEV_BUTTON_Y  BTN_NORTH  // Typically 'Y' or 'Triangle'
#define EVDEV_BUTTON_SEL BTN_SELECT
#define EVDEV_BUTTON_ST BTN_START
#define EVDEV_BUTTON_DOWN BTN_DPAD_DOWN
#define EVDEV_BUTTON_RIGHT BTN_DPAD_RIGHT
#define EVDEV_BUTTON_UP BTN_DPAD_UP
#define EVDEV_BUTTON_LEFT BTN_DPAD_LEFT

struct { unsigned short bit; int code; } buttons[] = {
    {A_BUTTON_MASK, EVDEV_BUTTON_A},
    {B_BUTTON_MASK, EVDEV_BUTTON_B},
    {X_BUTTON_MASK, EVDEV_BUTTON_X},
    {Y_BUTTON_MASK, EVDEV_BUTTON_Y},
    {SEL_BUTTON_MASK, EVDEV_BUTTON_SEL},
    {ST_BUTTON_MASK, EVDEV_BUTTON_ST},
    {UP_BUTTON_MASK, EVDEV_BUTTON_UP},
    {DOWN_BUTTON_MASK, EVDEV_BUTTON_DOWN},
    {LEFT_BUTTON_MASK, EVDEV_BUTTON_LEFT},
    {RIGHT_BUTTON_MASK, EVDEV_BUTTON_RIGHT},

};

typedef enum {
    DATA_COLLECT = 1,
    DATA_OUTPUT
} State_t;

void print_binary(uint16_t value) {
    for (int i = 9; i >= 0; i--) {
        // Check if the i-th bit is set and print 1 or 0
        putchar((value & (1 << i)) ? '1' : '0');
    }
    printf("\n");
}

// 16 bits 1 bit per button, when pressed the bit will be set to 1
// variable will be populated by readGpio()
int16_t input_data = 0x0000;
State_t state = DATA_COLLECT;

int setup_virtual_device(const char *name) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Error opening /dev/uinput. Do you have root privileges?");
        return -1;
    }

    // Enable all event types (EV_KEY is for buttons)
    ioctl(fd, UI_SET_EVBIT, EV_KEY);

    // Enable the specific buttons we want to simulate
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_A);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_B);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_X);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_Y);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_SEL);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_ST);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_UP);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_DOWN);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_LEFT);
    ioctl(fd, UI_SET_KEYBIT, EVDEV_BUTTON_RIGHT);

    // Create the device structure:
    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "%s", name);
    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor  = 0x1234; // Arbitrary Vendor ID
    uidev.id.product = 0x5678; // Arbitrary Product ID
    uidev.id.version = 1;

    if (write(fd, &uidev, sizeof(uidev)) < 0) {
        perror("Error writing device structure");
        close(fd);
        return -1;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("Error creating uinput device");
        close(fd);
        return -1;
    }

    printf("Virtual device created: %s\n", name);
    return fd;
}

void destroy_virtual_device(int fd) {
    if (fd >= 0) {
        ioctl(fd, UI_DEV_DESTROY);
        close(fd);
        printf("Virtual device destroyed.\n");
    }
}

void transitionState(State_t next){
    state = next;
}

// Function to send a single event to the virtual device
static int emit_event(int fd, int type, int code, int value) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.type = type;
    ev.code = code;
    ev.value = value;

    if (write(fd, &ev, sizeof(ev)) < 0) {
        perror("Error writing event");
        return -1;
    }
    return 0;
}

//This will be done latter
//Each Gpio state will be written to a bit using a MASK
void readGpio() {
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

void processData(int fd) {
    static uint16_t previous_data = 0x0000;

    unsigned short changed_bits = input_data ^ previous_data;

    //not optmizing this to keep reading time constant
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
        unsigned short mask = buttons[i].bit;
        if (changed_bits & mask) {
            int button_is_pressed = (input_data & mask) != 0;

            emit_event(fd, EV_KEY, buttons[i].code, button_is_pressed ? 1 : 0);
        }
    }
    // After sending all key events, we must send an EV_SYN report to signal
    // to the kernel that a complete state change has been sent.
    emit_event(fd, EV_SYN, SYN_REPORT, 0);

    previous_data = input_data;
}

void loop() {
    int fd = setup_virtual_device("Simple Virtual Gamepad");
    if (fd < 0) {
        return;
    }

    bool running = true;
    while (running) {
        switch (state) {
            case DATA_COLLECT:
                readGpio();
                transitionState(DATA_OUTPUT);
                break;
            case DATA_OUTPUT:
                processData(fd);
                print_binary(input_data);
                transitionState(DATA_COLLECT);
                break;
            default:
                running = false;
                break;
        }
        usleep(LOOP_INTERVAL);
    }
}


