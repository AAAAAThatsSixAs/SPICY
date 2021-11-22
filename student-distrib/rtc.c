#include "rtc.h"
#include "types.h"
#include "lib.h"



/* init_rtc
DESC:  Initializes the rtc
INPUT: none
OUTPUT: none
SIDE EFFECTS:Sends IRQ to PIC. and pushes appropriate values to ports
*/

static uint8_t RTC_IRQ_SLAVE=8; //the irq is set to 8 
static uint8_t RTC_IRQ_MASTER=2; // slave is connected to 2
void init_rtc()
{
	//diable interrupts
	cli();
	//disable irq
	disable_irq(RTC_IRQ_SLAVE);
	disable_irq(RTC_IRQ_MASTER);
	//output reg B
	outb(REGISTER_B, RTC_PORT);
	unsigned char previous=inb(RTC_DATA);//gets data from the data port
	outb(REGISTER_B, RTC_PORT);
	outb(previous|CHANNEL_0, RTC_DATA);		//or'd with the number specified by OSDEV
	//enable irq
	enable_irq(RTC_IRQ_MASTER);
	enable_irq(RTC_IRQ_SLAVE);
	//enbable interrupts
	sti();

}

/* rtc_interrupt
DESC:  sends data to the appropriate ports when the interrupt is called
INPUT: none
OUTPUT: none
SIDE EFFECTS:Sends data to ports.
*/
void rtc_interrupt()
{

	cli();
	outb(REGISTER_C, RTC_PORT);
	inb(RTC_DATA);


	/*uncomment this to test interrupts handled by the rtc
	  however doing so will cause a compilation warnings*/
	//test_interrupts();		
	//send_eoi(RTC_IRQ_MASTER);
	//used for checkpoint 2
	// if (rtcon)
		// printf("1");
	//set interrupt flag to 1
	intr_flag = 1;

	send_eoi(RTC_IRQ_SLAVE);
	sti();


}

/* rtc_open
DESC:  returns 0 when open() is called on the RTC
INPUT: none
OUTPUT: none
SIDE EFFECTS:none
*/
int rtc_open(const uint8_t* filename)
{
	return 0;
}

/* rtc_read
DESC:  returns 0 after an RTC interrupt has occured
INPUT: none
OUTPUT: none
SIDE EFFECTS:none
*/		
int rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
	//set flag to 0
	intr_flag = 0;
	//wait until rtc interrupt has occured (flag = 1)
	while(intr_flag == 0);

	return 0;

}

void sleep_rtc(uint32_t wait_time) {
	// ready = 0;

	int i;
	//wait
	for (i = 0; i < wait_time; i++) {
		rtc_read(0, 0, 0);
	}
	// ready = 1;
	intr_flag=1;

	return;

}


/* rtc_open
DESC:  sets the frequency of the RTC from 2Hz to 1024Hz
INPUT: buf - pointer to buffer containing 4 byte frequency
	   nbytes - nbytes read
OUTPUT:  0 on succesful frequncy change
		-1 on failure
SIDE EFFECTS: changes the RTC frequncy
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{

	//rate variable
	int rate = 0;

	//check if nbytes and buf are valid
	if (nbytes != sizeof(int))
		return -1;
	if (buf == NULL)
		return -1;

	//copy frequency to another var
	int freq = *((int*)buf);

	//check if rtc is within range 1 - 1024 (min and max frequencies)
	if (freq < 1 || freq > 1024)
		return -1;

	//check if valid power of 2
	if (freq & (freq - 1))
		return -1;
	
	//find rate cooresponding to frequency
	switch(freq)
	{
		case 1024:
			rate = F1024;
			break;
		case 512:
			rate = F512;
			break;
		case 256:
			rate = F256;
			break;	
		case 128:
			rate = F128;
			break;
		case 64:
			rate = F64;
			break;
		case 32:
			rate = F32;
			break;
		case 16:
			rate = F16;
			break;
		case 8:
			rate = F8;
			break;
		case 4:
			rate = F4;
			break;
		case 2:
			rate = F2;
			break;
		default:
			return -1;
	}


	//diable interrupts
	cli();
	//set register A
	outb(REGISTER_A, RTC_PORT);
	unsigned char prev = inb(RTC_DATA);

	//set reg A and send new freq data
	outb(REGISTER_A, RTC_PORT);
	outb((prev & 0xF0) | rate, RTC_DATA);
	//enable interrupts
	sti();

	return 0;
}

/* rtc_close
DESC:  returns 0 when close() is called on the RTC
INPUT: none
OUTPUT: none
SIDE EFFECTS:none
*/
int rtc_close(int32_t fd)
{
	return 0;
}
