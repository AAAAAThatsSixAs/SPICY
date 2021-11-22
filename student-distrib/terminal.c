#include "terminal.h"

int32_t terminal_open(const uint8_t* filename)
{
	return 0;
}

int32_t terminal_close(int32_t fd)
{
	return 0;
}

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
	//terminal buffer into buf
	//returns number of bytes read
	//can manually add this enter.
	while(1)
	{
		if(enterFlag==1)
		{
			enterFlag=0;
			break;
		}
	}

	uint8_t totSize;
	totSize=0;

	if(buffer[bufferIndex-1]!='\n')
		totSize=1;

	totSize+=nbytes;
	if(bufferIndex<nbytes)
	{
		totSize+=bufferIndex;
	}
	memcpy(buf, buffer, totSize);
	if(*(unsigned char*)(buf+totSize-1) !='\n')
	{
		*(unsigned char*)(buf+totSize-1) ='\n';
		*(unsigned char*)(buf+totSize) ='\0';
	}
	bufferIndex = 0;
	memset(buffer, 0, BUFFER_MAX);

return totSize;
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{		
	uint32_t i;
	for (i=0 ; i < nbytes; i++)
	{
		putc(*(unsigned char*)(buf+i));
	}
	return i;
}

/* init_terminal
* Desc: Initialize the three terminals 
* INPUTS: none
* OUTPUTS: none
* Side Effects: starts three terminal processes and begins first terminal and shell process
**/
void init_terminal() {
	cli();
	int i;
	//initialize terminal struct entries
	for (i = 0; i < _three; i++) {
		terminals[i].p_num = 1;
		terminals[i].esp = '\0';
		terminals[i].ebp = '\0';
		terminals[i].active = 0;
		terminals[i].isfirst = 1;
		terminals[i].bufindex = 0;
		terminals[i].screen_x = 0;
		terminals[i].screen_y = 0;
		int j;
		for (j = 0; j < BUFFER_MAX; j++) {
			terminals[i].buf[j] = '\0';
		}
		// initialize video memory virtual spaces 
		terminals[i].vmem_ptr = (uint32_t *)((char *)VIDMEM_TERMINAL_LOC + (i * _4KB));
		// map pages to their respective physical addresses
		user_virt_to_phys(START_FREE_MEM + _4KB*(i), VIDMEM_TERMINAL_LOC + (i * _4KB), i);
		//map each three virtual buffers to their corresponding physical buffers 
		// set buffer to the new terminal buffer
	}

	for(i = 0; i < 6; i++)
		pid_list[i] = 0;

	curr_term_disp_num = 0;
	// printf("%s\n", __DATE__);
	// printTerminal(1);
	// char a[128];
	// char b[128];
	// userpass(a, b);
}

/* start_terminal
* Desc: takes a number and begins the terminal associated with it
* INPUTS: terminal_num - int corresponding to terminal number
* OUTPUTS: none
* Side Effects: starts terminal process and replaces all associated globals with terminal's data
**/
void start_terminal(int terminal_num) {

	//set buffer to current terminal buffer


   		terminals[terminal_num].active = 1;
   		int32_t cesp;
   		int32_t cebp;
   			asm volatile(
				"movl %%ebp, %0;"
				"movl %%esp, %1;"
				:"=r"(cebp), "=r"(cesp)
				: /*no inputs*/
				: "memory", "cc"
				);
		terminals[curr_term_disp_num].esp = cesp;
		terminals[curr_term_disp_num].ebp = cebp;
		uint32_t* temp = (uint32_t*)curr_pcb;
		terminals[curr_term_disp_num].curr_pcb_ptr = temp;
   	
	buffer = terminals[terminal_num].buf;
	bufferIndex = terminals[terminal_num].bufindex;

	curr_term_disp_num = terminal_num;
	//save esp and ebp
	flush_tlb();

	/*====================================================================================*/
	memcpy((void *)V_MEM_PAGE_LOC, (void *)terminals[curr_term_disp_num].vmem_ptr, 2*NUM_ROWS*NUM_COLS);
	flush_tlb();
	sti();
	send_eoi(1);
	execute((uint8_t *)"shell");

	/* run the first shell of the terminals
	asm volatile(
	"int $0x80;"
	:
	:"a"(2),"b"("shell")
	:"memory", "cc"
	);
	*/
}

/* change_terminal
* Desc: saves terminal state and changes to a new one
* INPUTS: next_term_num terminal number to switch to
* OUTPUTS: none
* Side Effects: starts terminal process and replaces all associated globals with terminal's data
**/
void change_terminal(int curr_term_num, int next_term_num) {
	//check if the terminal to switch to is on
	cli();
	if (curr_term_num == next_term_num)
	{
		send_eoi(1);
		sti();
		printf("%s%d%c", "Already in terminal ", next_term_num, '\n');
		return;
	}
	if (terminals[next_term_num].active == 0) {
		printf("%s", "_SPICYOS> ");
		printf("%s%d%c", "Starting terminal ", next_term_num, '\n');
		start_terminal(next_term_num);
	}
	else {
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
		terminals[curr_term_num].bufindex = bufferIndex;
		//restore other terminals esp and ebp
		//set new keyboard buf and pcb
		buffer = terminals[next_term_num].buf;
		bufferIndex = terminals[next_term_num].bufindex;

		curr_pcb = (pcb_t *)terminals[next_term_num].curr_pcb_ptr;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = SHELL_LOCATION - (KSTACK_SIZE * curr_pcb->ID)-4;
		//remap terminal stuff
		virt_to_phys(SHELL_LOCATION + (curr_pcb->ID)*_4MB, VMEM_LOCATION);
		flush_tlb();
		curr_term_disp_num = next_term_num;

		//v_mem_page_loc
		//copy data from current video memory to virtual location of current terminal pointer
		/*====================================================================================*/
		//copy video buffer to current process physical buffer
		memcpy((void *)terminals[curr_term_num].vmem_ptr, (void *)V_MEM_PAGE_LOC, _4KB);
		//copy next process buffer to video buffer
		memcpy((void *)V_MEM_PAGE_LOC, (void *)terminals[next_term_num].vmem_ptr, _4KB);

		printf("%s%d%c", "Now in terminal ", next_term_num, '\n');
		printf("%s", "_SPICYOS> ");

		asm volatile(
		"mov %0, %%ebp;"
		: /*Outputs*/
		: "r"(terminals[next_term_num].ebp)
		: "memory"/*Clobbered Registers*/
		);
		send_eoi(1);
		sti();
		return;
	}
	return;
}
