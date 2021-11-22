#ifndef SCHEDULING_H
#define SCHEDULING_H

#ifndef ASM

#include "terminal.h"
//define PIT ports
#define PIT_IRQ_0 0
#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT 0x40

//define PIT commands to send
#define PIT_SQUARE_WAVE 0x36 //channel 0, lobyte/hibyte, square, 16bit binary
#define FREQUENCY_10Hz 1193180/2
#define SHIFT_8 8
#define PIT_MASK 0xFF
#define NUM_TERMS 3

//terminal number to schedule
uint32_t next_term_ID;
uint32_t to_schedule;


//function definitions
void init_pit();
void pit_interrupt();
uint32_t find_next_task();
uint32_t context_switch(uint32_t next_term_num);

#endif

#endif


