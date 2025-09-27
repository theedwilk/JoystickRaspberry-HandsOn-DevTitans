#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#define LOOP_INTERVAL 10000
#define B_BUTTON_MASK 1
#define A_BUTTON_MASK 1 << 1
#define Y_BUTTON_MASK 1 << 2
#define X_BUTTON_MASK 1 << 3
#define SEL_BUTTON_MASK 1 << 4
#define ST_BUTTON_MASK 1 << 5
#define FIRST_BIT_MASK 1

typedef enum {
    BTN_B = 0,
    BTN_A,
    BTN_Y,
    BTN_X,
    BTN_SEL,
    BTN_ST,
    END_READ
} BtnReadingOrder_t;

typedef enum {
    DATA_COLLECT = 1,
    DATA_OUTPUT
} State_t;

// 8 bits 1 bit per button, when pressed the bit will be set to 1
// variable will be populated by readGpio()
int8_t input_data = 0;

State_t state = DATA_COLLECT;

void transitionState(State_t next){
    state = next;
}

//This will be done latter
//Each Gpio state will be written to a bit using a MASK
void readGpio() {
    input_data |= A_BUTTON_MASK;
    input_data |= X_BUTTON_MASK;
}

void processData() {
    BtnReadingOrder_t curr = BTN_B;

    //not optmizing this to keep reading time constant
    while (curr != END_READ) {
        bool bit = input_data & FIRST_BIT_MASK;
        printf("bit %d is set to %d\n",curr,bit);
        //do whatever needs to be done here;
        input_data = input_data >> 1;
        curr++;
    }
}

void loop() {
    bool running = true;
    while (running) {
        switch (state) {
            case DATA_COLLECT:
                readGpio();
                transitionState(DATA_OUTPUT);
                break;
            case DATA_OUTPUT:
                processData();
                transitionState(DATA_COLLECT);
                break;
            default:
                running = false;
                break;
        }
        usleep(LOOP_INTERVAL);
    }
}


