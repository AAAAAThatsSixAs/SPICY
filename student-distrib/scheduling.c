#include "lib.h"
#include "scheduling.h"
#include "i8259.h"

/* init_pit
DESC:  Initializes the pit
INPUT: none
OUTPUT: none
SIDE EFFECTS:Sends IRQ to PIT. and pushes appropriate values to ports
*/
void init_pit()
{

	next_term_ID = 0;
	to_schedule = 0;

	//diable interrupts
	cli();
	//disable irq
	disable_irq(PIT_IRQ_0);

	//referenced from osdev: https://github.com/stevej/osdev/blob/master/kernel/devices/timer.c
	//output square wave mode
	outb(PIT_SQUARE_WAVE, PIT_COMMAND_PORT);
	//send frequency data
	outb(FREQUENCY_10Hz & PIT_MASK, PIT_DATA_PORT);
	//send frequency data shifted 8 bits
	outb((FREQUENCY_10Hz >> SHIFT_8) & PIT_MASK, PIT_DATA_PORT);	

	//enable irq
	enable_irq(PIT_IRQ_0);
	//enbable interrupts
	sti();

	return;

}

/* pit_interrupt
DESC:  interrupt handler for pit, context switch if necessary
INPUT: none
OUTPUT: none
SIDE EFFECTS:
*/
void pit_interrupt()
{
	

	//send eoi
	send_eoi(PIT_IRQ_0);
	//clear intterupts
	cli();
	
	find_next_task();

	//context switch
	if (to_schedule)
		context_switch(next_term_ID);

	sti();

	return;
}


/* find_next_task
DESC:  helper function to find next task
INPUT: none
OUTPUT: none
SIDE EFFECTS:none
*/
uint32_t find_next_task()
{
	//check if only terminal 0 is active
	if (terminals[1].active == 0 && terminals[2].active == 0)
	{
		//do not schedule flag
		to_schedule = 0;
		return 0;
	}

	//schedule 
	to_schedule = 1;

	//find the next terminal to schedule, round robin style
	while(1)
	{
		next_term_ID ++;
		next_term_ID = next_term_ID % NUM_TERMS;
		if (terminals[next_term_ID].active == 1)
			break;
	}

	//printf("next term num %d\n", next_term_ID);

	return next_term_ID;
}

/* context switch
DESC: switch to program running in the next terminal, 
	stores ebp, esp, remaps memory and replaces associated registers
INPUT: none
OUTPUT: none
SIDE EFFECTS:
*/
uint32_t context_switch(uint32_t next_term_num)
{
	int32_t curr_term_num = curr_term_disp_num;
	int32_t curr_esp;
	int32_t curr_ebp;
	asm volatile(
				"movl %%ebp, %0;"
				"movl %%esp, %1;"
				:"=r"(curr_ebp), "=r"(curr_esp)
				:
				: "memory", "cc"
	);

	terminals[curr_term_num].esp = curr_esp;
	terminals[curr_term_num].ebp = curr_ebp;


	uint32_t* temp = (uint32_t*)curr_pcb;
	terminals[curr_term_num].curr_pcb_ptr = temp;
	//restore other terminals esp and ebp

	//set new keyboard buf and pcb
	curr_pcb = (pcb_t *)terminals[next_term_num].curr_pcb_ptr;
	tss.esp0 = SHELL_LOCATION - (KSTACK_SIZE * curr_pcb->ID)-4;
	//remap terminal stuff
	//user_virt_to_phys(SHELL_LOCATION + (curr_pcb->ID)*_4MB, VMEM_LOCATION, next_term_num);	
	flush_tlb();
	curr_term_disp_num = next_term_num;
	send_eoi(1);

	// this was in JD's ec branch
	//set new pcb
	// curr_pcb = (pcb_t *)terminals[next_term_num].curr_pcb_ptr;
	// tss.esp0 = SHELL_LOCATION - (KSTACK_SIZE * curr_pcb->ID)-4;
	//remap terminal stuff
	// user_virt_to_phys(SHELL_LOCATION + (curr_pcb->ID)*_4MB, VMEM_LOCATION, next_term_num);	
	// flush_tlb();
	// curr_term_disp_num = next_term_num;

	//printf("%s%d%c", "Now in terminal ", curr_term_disp_num, '\n');
	//printf("%s", "_SPICYOS> ");
	asm volatile(
	"mov %0, %%esp;"
	"mov %1, %%ebp;"
	: /*Outputs*/
	: "r"(terminals[next_term_num].esp), "r"(terminals[next_term_num].ebp)
	: "memory"/*Clobbered Registers*/
	);
	sti();
	return 0;
}




