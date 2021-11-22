//keyboard.h


#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifndef ASM

#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "rtc.h"
#include "filesystem.h"
#include "terminal.h"
#include "sound.h"
#include "memory.h"
#include "valgrind.h"


extern void pagefault(char* err);
//from osdev

#define NUM_COLS 80
#define NULL_KEY 0
#define Q_PRESSED 0x10
#define Q_RELEASED 0x90
#define W_PRESSED 0x11
#define W_RELEASED 0x91
#define E_PRESSED 0x12
#define E_RELEASED 0x92
#define R_PRESSED 0x13
#define R_RELEASED 0x93
#define T_PRESSED 0x14
#define T_RELEASED 0x94
#define Y_PRESSED 0x15
#define Y_RELEASED 0x95
#define U_PRESSED 0x16
#define U_RELEASED 0x96
#define I_PRESSED 0x17
#define I_RELEASED 0x97
#define O_PRESSED 0x18
#define O_RELEASED 0x98
#define P_PRESSED 0x19
#define P_RELEASED 0x99
#define SBRACKET_PRESSED 0x1B



#define A_PRESSED 0x1E
#define A_RELEASED 0x9E
#define S_PRESSED 0x1F
#define S_RELEASED 0x9F
#define D_PRESSED 0x20
#define D_RELEASED 0xA0
#define F_PRESSED 0x21
#define F_RELEASED 0xA1
#define G_PRESSED 0x22
#define G_RELEASED 0xA2
#define H_PRESSED 0x23
#define H_RELEASED 0xA3
#define J_PRESSED 0x24
#define J_RELEASED 0xA4
#define K_PRESSED 0x25
#define K_RELEASED 0xA5
#define L_PRESSED 0x26
#define L_RELEASED 0xA6
#define BTICK_PRESSED 0x29



#define Z_PRESSED 0x2C
#define Z_RELEASED 0xAC
#define X_PRESSED 0x2D
#define X_RELEASED 0xAD
#define C_PRESSED 0x2E
#define C_RELEASED 0xAE
#define V_PRESSED 0x2F
#define V_RELEASED 0xAF
#define B_PRESSED 0x30
#define B_RELEASED 0xB0
#define N_PRESSED 0x31
#define N_RELEASED 0xB1
#define M_PRESSED 0x32
#define M_RELEASED 0xB2
#define SLASH_PRESSED 0x35

#define L_SHIFT_PRESSED 0x2A
#define R_SHIFT_PRESSED 0x36
#define CAPS_PRESSED 0x3A

#define L_SHIFT_RELEASED 0xAA
#define R_SHIFT_RELEASED 0xB6

#define ZERO_PRESSED 0x0B
#define ONE_PRESSED 0x2
#define TWO_PRESSED 0x3
#define THREE_PRESSED 0x4
#define FOUR_PRESSED 0x5
#define FIVE_PRESSED 0x6
#define SIX_PRESSED 0x7
#define EQUAL_PRESSED 0xD

#define POINT_PRESSED 0x34
#define POINT_RELEASED 0xB4

#define BSLASH_PRESSED 0x2B
#define SLASH_RELEASED 0xB5

#define BACKSPACE_PRESSED 0xE
#define BACKSPACE_RELEASED 0x8E
#define SPACE_PRESSED 0x39
#define SPACE_RELEASED 0xB9
#define ENTER_PRESSED 0x1C
#define ENTER_RELEASED 0x9C

#define L_CTRL_PRESSED 0x1D
#define R_CTRL_PRESSED 0xE0
#define L_CTRL_RELEASED 0x9D
#define R_CTRL_RELEASED 0x9D
#define ALT_PRESSED 0x38
#define ALT_RELEASED 0xB8

#define F1_PRESSED 0x3B
#define F2_PRESSED 0x3C
#define F3_PRESSED 0x3D

#define ENABLE_KEYBOARD 1
#define KEYBOARD_DATA_PORT 0x60
#define BUFFER_MAX 128		//max size of char
#define KEYBOARD_INT_PORT 0x21




//characters
char *buffer;	//buffer to print character to screen

uint8_t bufferIndex;//this is #char \s printed
uint8_t shiftFlag;
int capsFlag;
uint8_t ctrlFlag;
uint8_t passwordFlag;
uint8_t altFlag;
volatile uint8_t enterFlag;

//function headers
uint8_t asciiConversion (uint8_t cmd);

uint8_t asciiConversionShift (uint8_t cmd);

uint8_t asciiConversionCaps (uint8_t cmd);

uint8_t asciiConversionShiftCaps (uint8_t cmd);

void init_keyboard(void);

void keyboard_interrupt(void);

#endif //ASM
#endif //KEYBOARD_H

