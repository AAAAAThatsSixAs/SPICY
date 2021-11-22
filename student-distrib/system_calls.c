#include "system_calls.h"

/* halt
DESC: a system call that terminates a proess
INPUT: status, an unsigned integer indicating type of interrupt
OUTPUT: none	
SIDE EFFECTS:
*/


int32_t halt (uint8_t status) {
	cli();
	int islast = 0;
	int i;
	for (i = 0; i < MAX_NUM_PROCESS; ++i)
	{
		islast += pid_list[i];
	}
	if (islast <= 1 || process_num[curr_term_disp_num] == 1)
	{
		process_num[curr_term_disp_num]--;
		printf("Cannot halt last program u dingus\n");
		//run another shell
		pid_list[curr_pcb->ID] = 0;
		execute((uint8_t *) "shell");
	}
	else {
		pid_list[curr_pcb->ID] = 0;
		process_num[curr_term_disp_num]--;
		//Return curr process pcb to parent process
	}
	//reset FD array
	for (i = FDMIN; i < FDMAX; i++)
	{
		curr_pcb->open_files[i].fileops_tableptr = NULL;
		curr_pcb->open_files[i].inode = 0;
		curr_pcb->open_files[i].file_loc = 0;
		curr_pcb->open_files[i].flags = 0;
	}
	virt_to_phys(SHELL_LOCATION + (curr_pcb->parent_ID)*_4MB, VMEM_LOCATION);	
	flush_tlb();
	
	//restore parent ebp and esp
	tss.esp0 = SHELL_LOCATION - (KSTACK_SIZE * curr_pcb->parent_ID)-4;
	//tss.esp0 = 0x7ffffc;
	tss.ss = KERNEL_DS;

	//set pcb to parent pcb
	uint32_t p_esp = curr_pcb->saved_esp;
	uint32_t p_ebp = curr_pcb->saved_ebp;

	curr_pcb = (pcb_t*) curr_pcb->parent_ptr;
	sti();

	setScreenX(0);
	//unmap(process_num);
	//restore esp and ebp to parent program and return
	//HALT_RETURN returns to bottom of execute()
	sti();
	asm volatile(
		"mov %0, %%esp;"
		"mov %1, %%ebp;"
		"mov %2, %%eax;"
		"jmp HALT_RETURN"
		: /*Outputs*/
		: "r"(p_esp), "r"(p_ebp), "r"((uint32_t)status)
		: "%eax"/*Clobbered Registers*/
		);
	return 0;
}

/* execute
DESC: executes a user level program
INPUT: 
OUTPUT:
SIDE EFFECTS:
*/
int32_t execute (const uint8_t* command) {

	// play_music();
	cli();

	
	//argument parsing
	int i = 0;
	uint8_t _command[ (uint32_t) strlen((int8_t*)command)]; 

	int q;
	for(q=0; q< MAX_CMD_LENGTH; q++)
		_command[q] = '\0';


	memcpy((uint32_t*)_command, command, (uint32_t)strlen((int8_t*)command));

	if (command[i] == ' ')
	{
		while (command[i] == ' ' && i <=32) {
			i++;
		}
	}

	int start_index = i;
	//count number of arguments passed and take care of trailing spaces
	while (command[i] != '\0' && i < 32) {
		if(command[i] != '\n')
			break;
		i++;
	}
	while (command[i-1] == ' ') {
		i--;
	}
	//int end_index = i;

	uint8_t filename[FILENAME_SIZE + 1]; 
	int c;
	for (c = 0; c<FILENAME_SIZE + 1; c++) {
		filename[c] = '\0';
	}
	i = 0;
	while (command[start_index] != ' ' && command[start_index] != NULL && i < 64)
	{
		//get filename 
		filename[i] = command[start_index];
		start_index++;
		i++;
	}
	//null terminate the filename
	filename[i] = '\0';
	i = 0;
	
	/*check file validity*/
	//valid file name
	dentry_t file_test;
	int t = read_dentry_by_name(filename, &file_test);
	if (t == -1)
		return -1; //fail if read_dentry fails

	//valid executable
	uint8_t buf[BUF_SIZE];
	read_data(file_test.inode_index, 0, buf, BUF_SIZE);

	if(buf[0] != MAGIC_1 ||
	   buf[1] != MAGIC_2 ||
	   buf[2] != MAGIC_3 ||
	   buf[3] != MAGIC_4)
    	return -1; //fail if one of the four magic bytes 
    //(denoting an executable) is not correct

    process_num[curr_term_disp_num]++;

    //check if too many processes
    /*
	possible conditions
	all terms off 		sum = 0
	t0 on, t1+t2 off   	sum = 1
	t0+tx on, ty off 	sum = 2
	t0+t1+t2 on 		sum = 3
	n number of user programs + previous conditions
						sum = n + 1/2/3
    */
    //sum is the total number of processes running including base shell for each terminal
	int new_pid;
	new_pid = find_new_pid();
    if (new_pid == -1) //MAX_NUM_PROCESS = 6
    {
    	return -1; //fail if too many processes
    }
    pid_list[new_pid] = 1;
 
    int offset = _4MB * new_pid;
    virt_to_phys(SHELL_LOCATION + offset, VMEM_LOCATION);

    //maps the physical to virtual mem
    inode_entry_t* inode_entry = (inode_entry_t*)(FILESYS_START + (file_test.inode_index + 1) * BLOCK_SIZE);
    //uint8_t inodebuf[inode_entry.size];
    flush_tlb();
    read_data(file_test.inode_index, 0, (uint8_t*)VMEM_OFFSET, inode_entry->size);//copy into 080480000

    //obtain entry point of file (assuming valid)
    read_data(file_test.inode_index, TWENTY_FOUR, (uint8_t*)buf, BUF_SIZE); //24-27 bytes are the starting point
    uint32_t *program_entry_addr = (uint32_t *)(*((uint32_t*)buf));
   	//Create PCB

   	//set process number and check if it is the first one
   	if (terminals[curr_term_disp_num].isfirst)
   	{
   		pcb_t * first_pcb = (pcb_t*)(SHELL_LOCATION - (new_pid+1)*KSTACK_SIZE);
   		//if process is the first one then set parent process to be null
   		uint32_t* temp = (uint32_t*)first_pcb;
   		(first_pcb->parent_ptr) = temp; //set parent to itself
   		first_pcb->parent_ID = new_pid;
   		first_pcb->ID = new_pid;
   		curr_pcb = first_pcb;
   		//remove first flag
   		terminals[curr_term_disp_num].isfirst = 0;
   	}
   	else {
   		//check for parent pointer and ID and set accordingly
   		//increment process number
   	   	pcb_t * new_pcb =(pcb_t*)(SHELL_LOCATION - (new_pid+1)*KSTACK_SIZE);
   		uint32_t* temp = (uint32_t*)curr_pcb;
   		(new_pcb->parent_ptr) = temp; //set parent to curr
   		new_pcb->parent_ID = curr_pcb->ID;
	   	new_pcb->ID = new_pid;
	   	curr_pcb = new_pcb;
   	}

   	int x;
   	for (x = 0; x < MAX_CMD_LENGTH; x++)
   		(curr_pcb->args)[i] = '\0';

   	while (_command[start_index] != '\0')
	{
		start_index++;
		(curr_pcb->args)[i] = _command[start_index];
		i++;
	}


	//fd stuff
   	init_file_array();
   	/*
   	for (i = 0; i < 20; i++)
   	{
   		dentry_t* d = (dentry_t*)(FILESYS_START + (i+1) * DENTRY_SIZE);
   		printf("inode entry: %d      name:", d->inode_index);
   		printf("%s\n", (char*)(FILESYS_START + (i+1) * DENTRY_SIZE));
   	}
   	*/
	

   	//context switch
   	tss.ss0 = KERNEL_DS;
   	//kernel shell location
   	tss.esp0 = SHELL_LOCATION - (KSTACK_SIZE * curr_pcb->ID)-4;


   	//update terminal struct with pcb
   	terminals[curr_term_disp_num].curr_pcb_ptr = (uint32_t *)curr_pcb; 

	//save esp in PCB
	uint32_t esp_r;
	uint32_t ebp_r;
	asm volatile(
		"movl %%esp, %0;"
		"movl %%ebp, %1;"
        : "=r"(esp_r), "=r"(ebp_r)
        : //no inputs
        : "%eax", "%ebx"
        );
   	curr_pcb->saved_esp = esp_r;
   	curr_pcb->saved_ebp = ebp_r;


   	int halt_retval = 0;

   	sti();
   	asm volatile(
			".globl HALT_RETURN;"
             "cli;" //prevent interrupts from messing this up
             "mov $0x2B, %%ax;"
             "mov %%ax, %%ds;" //put x2B in ds  
 			 "mov %%ax, %%es;" 
			 "mov %%ax, %%fs;"
			 "mov %%ax, %%gs;"   

             "pushl $0x2B;" //user mode data selector
             "movl $0x83FFFFC, %%eax;"//push user stack at 132 MB-1 (0x83FFFFC)
             "pushl %%eax;"

             //set flags
             "pushfl;"
             "popl %%edx;"
             "orl $0x200, %%edx;" //sets IF in EFLAGS so interrupts are enabled
             "pushl %%edx;"

             //push code segment 
             "pushl $0x23;"
             "pushl %%ebx;" //push program_entry_addr (stored in register 0)

             "iret;"

             "HALT_RETURN:"

             "movl %%eax, %0;"
             //"movl %2, %%ecx;"
             :"=r" (halt_retval)	/* outputs */
             :"b"(program_entry_addr)
             :"%edx","%eax"	/* clobbered register */
             ); //halt leave ret label when more than one program running

	return halt_retval;
}

uint32_t init_file_array()
{

	//setup rtc jumptable
	rtc_fileops.open_ptr = rtc_open;
	rtc_fileops.read_ptr = rtc_read;
	rtc_fileops.write_ptr = rtc_write;
	rtc_fileops.close_ptr = rtc_close;

	//setup file jumptable
	file_fileops.open_ptr = file_open;
	file_fileops.read_ptr = file_read_sys;
	file_fileops.write_ptr = file_write;
	file_fileops.close_ptr = file_close;

	//setup directory jumptable
	dir_fileops.open_ptr = dir_open;
	dir_fileops.read_ptr = dir_read_sys;
	dir_fileops.write_ptr = dir_write;
	dir_fileops.close_ptr = dir_close;

	//stdin terminal jumptable has 
	stdio_fileops.open_ptr = terminal_open;
	stdio_fileops.read_ptr = terminal_read;
	stdio_fileops.write_ptr = terminal_write;
	stdio_fileops.close_ptr = terminal_close;

	int i;
	for (i = 0; i < FDMAX; i++)
	{
		if (i < FDMIN)
		{
			curr_pcb->open_files[i].fileops_tableptr = &stdio_fileops;
			curr_pcb->open_files[i].inode = 0;
			curr_pcb->open_files[i].file_loc = 0;
			curr_pcb->open_files[i].flags = 1;
		}
		else
		{
			curr_pcb->open_files[i].flags = 0;
		}
	}


	//clear
	for(i = 0; i < NUM_INODES; i++)
		used_inodes[i] = 0;
	for(i = 0; i < NUM_DATABLOCKS; i++)
		used_dbs[i] = 0;

	bootblock_header_t* bb = (bootblock_header_t*)FILESYS_START;
	last_directory = bb->num_dir;

	//mark used inodes and datablocks
	int j;
	int inode_used;

	for(i = 0; i < last_directory; i++)
	{
		dentry_t* de = (dentry_t*)(FILESYS_START + DENTRY_SIZE + i * DENTRY_SIZE);
		inode_used = de->inode_index;
		used_inodes[inode_used] = 1;

		inode_entry_t* inode_entry_used = (inode_entry_t*)(FILESYS_START + (inode_used + 1) * BLOCK_SIZE);

		for(j = 0; j < ((inode_entry_used->size)/4096) + 1 ; j++)
		{
			used_dbs[inode_entry_used->data[j]] = 1;
		}
	}

	/*

	printf("filesys start: %x \n", FILESYS_START);
	printf("numdirs: %d\n",*((int*)FILESYS_START) );
	printf("inodes: %d\n",*((int*)FILESYS_START+1) );
	printf("datablocks: %d\n",*((int*)FILESYS_START+2));

	printf("inodes used: ");
	for(i = 0; i < 64; i++)
		if (used_inodes[i] == 1)
			printf("%d ", i);

	printf("\n");

	printf("datablocks used: ");
	for(i = 0; i < 256; i++)
		if (used_dbs[i] == 1)
			printf("%d ", i);

	
	*/

return 0;

}


int32_t open(const uint8_t* filename)
{
	if (filename == NULL || *filename == '\0')
		return -1;
	int fd;
	for (fd = FDMIN; fd <= FDMAX; fd++)
	{

		//if none are found return -1
		if (fd == FDMAX) 
			return -1;


		//find open file descriptor
		if (curr_pcb->open_files[fd].flags == 0)
			break;

		
	}

	dentry_t file_to_find;

	if (read_dentry_by_name(filename, &file_to_find) == -1)
		return -1;

	//set to used
	curr_pcb->open_files[fd].flags = 1;


	//populate file_array 
	switch(file_to_find.type)
	{
		//file is RTC
		case 0:
			curr_pcb->open_files[fd].fileops_tableptr = &rtc_fileops;
			curr_pcb->open_files[fd].inode = 0;
			curr_pcb->open_files[fd].file_loc = 0;
			curr_pcb->open_files[fd].flags = 1;
			break;
		//file is Directory
		case 1:
			curr_pcb->open_files[fd].fileops_tableptr = &dir_fileops;
			curr_pcb->open_files[fd].inode = file_to_find.inode_index;
			curr_pcb->open_files[fd].file_loc = 0;
			curr_pcb->open_files[fd].flags = 1;
			break;
		//file is an actual file
		case 2:
			curr_pcb->open_files[fd].fileops_tableptr = &file_fileops;
			curr_pcb->open_files[fd].inode = file_to_find.inode_index;
			curr_pcb->open_files[fd].file_loc = 0;
			curr_pcb->open_files[fd].flags = 1;
			break;
	}

	//open the file
	(*(((curr_pcb->open_files[fd]).fileops_tableptr)->open_ptr))(filename);

	return fd;
}


int32_t close(int32_t fd)
{
	//if file is invalid
	if (fd < FDMIN || fd >= FDMAX)
		return -1;

	//if file does not exist
	if (curr_pcb->open_files[fd].flags == 0)
		return -1;

	//set flag bit to 0 to free file descriptor
	curr_pcb->open_files[fd].flags = 0;

	(*(curr_pcb->open_files[fd].fileops_tableptr->close_ptr))(fd);

	return 0;

}

int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
	//if file is invalid
	if (fd < 0 || fd > FDMAX || fd == 1)
		return -1;

	//if file does not exist
	if (curr_pcb->open_files[fd].flags == 0)
		return -1;

	//if buffer is invalid
	if (buf == NULL)
		return -1;

	int retval = (*(curr_pcb->open_files[fd].fileops_tableptr->read_ptr))(fd, buf, nbytes);

	if (retval != -1)
		(curr_pcb->open_files[fd]).file_loc += retval;

	return retval;
}


int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
	//if file is invalid
	if (fd < 0 || fd > FDMAX || fd == 0)
		return -1;

	//if file does not exist
	if (curr_pcb->open_files[fd].flags == 0)
		return -1;

	//if buffer is invalid
	if (buf == NULL)
		return -1;

	int retval = (*(curr_pcb->open_files[fd].fileops_tableptr->write_ptr))(fd, buf, nbytes);

	return retval;
}

int32_t getargs (uint8_t* buf, int32_t nbytes){
	memcpy(buf, curr_pcb->args, nbytes);
	return 0;
}
int32_t vidmap (uint8_t** screen_start) {
	//check location validity
	if ((uint32_t) screen_start < _128MB || (uint32_t) screen_start > _128MB + _4MB)
		return -1;
	*screen_start = (uint8_t*)(_128MB + _4MB);
	map_vid_mem_user();

	return (_128MB + _4MB);
}
int32_t set_handler (int32_t signum, void* handler_address) {
	return -1;
}
int32_t sigreturn (void) {
	return -1;
}

int32_t create(const uint8_t* filename)
{
	//check if filename is invalid
	if (filename == NULL)
		return -1;

	return file_create(filename);
}


int32_t remove(const uint8_t* filename)
{
	//check if filename is invalid
	if (filename == NULL)
		return -1;

	return file_remove(filename);
}

int32_t find_new_pid()
{
	int i;
	for(i = 0; i < MAX_NUM_PROCESS; i++)
	{
		if (pid_list[i] == 0)
			return i;
	}

	return -1;
}

void valgrind(void) {
	setTrackOn();
	play_music();

/*
	int i;
 	int* ptr;
 	// ptr = (int*) malloc(sizeof);
 	for (i = 1; i <= 391; i++) {
 		ptr = (int*) malloc(sizeof(int));
 		*ptr = i;
 		*ptr *= i;
 		printf("ptr = %d\n", *ptr); 
 		free(ptr);
 	}
*/
	printresults();
}
