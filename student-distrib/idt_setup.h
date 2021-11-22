/* idt_setup.h - setups the idt table */

#ifndef IDT_SETUP_H
#define IDT_SETUP_H


//define vector
#define KEYBOARD_PIC 0x21
#define RTC_PIC 	 0x28
#define SYSCALL_PIC 0x80
#define PIT_PIC		0x20

#ifndef ASM
#define ASM

int idtsetup(void);

//function headers from interrupted.S
//function header for rtc interrupt
extern void rtc_interrupted();
//function header for keyboard interrupts
extern void keyboard_interrupted();
//handler for system calls
extern void syscalls_interrupted();
//handler for pit
extern void pit_interrupted();

#endif //ASM

#endif //IDT_SETUP_H
