#ifndef RTC_H
#define RTC_H

#ifndef ASM

#include "types.h"
#include "i8259.h"
#include "lib.h"

//register addresses
#define REGISTER_A 0x8A
#define REGISTER_B 0x8B
#define REGISTER_C 0x0C
#define REGISTER_D 0x8D
//port addresses
#define RTC_PORT 0x70
#define RTC_DATA 0x71
#define CHANNEL_0 0x40

//define RTC rates
#define F2 		0xF
#define F4 		0xE
#define F8 		0xD
#define F16 	0xC
#define F32 	0xB
#define F64 	0xA
#define F128 	0x9
#define F256 	0x8
#define F512 	0x7
#define F1024 	0x6


//PIC
int rtcon;
void init_rtc();


//function headers
void rtc_interrupt();
extern void test_interrupts();
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int rtc_open(const uint8_t* filename);
int rtc_read(int32_t fd, void* buf, int32_t nbytes);
int rtc_close(int32_t fd);
void sleep_rtc(uint32_t wait_time);

//flag to see if interrup has occured
int intr_flag;




#endif //ASM
#endif //KEYBOARD_H
