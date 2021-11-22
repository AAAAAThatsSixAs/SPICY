#include "keyboard.h"





/* init_keyboard
DESC:  Initializes the keyboard
INPUT: none
OUTPUT: none
SIDE EFFECTS:Sends IRQ to PIC.
*/

int trigger; //written in all other ctrl functions
int index;
static char *username="user";
static char *password="pagefault";
extern uint8_t globalTextColor;
extern uint8_t globalBackgroundColor;


static char *numbers="1234567890-=";		//0x20 -> 0xD
static char *firstLetters="qwertyuiop[]"; //0x10 -> 0x1B
static char *secondLetters="asdfghjkl;'`"; //0x1E -> 0x29
static char *thirdLetters="zxcvbnm,./";	//0x2C -> 0x35

static char *numbersShift1="!@#$";		//0x20 -> 0xD
static char *numbersShift2="^&*()_+";		//0x20 -> 0xD
static char *firstLettersShift="QWERTYUIOP{}"; //0x10 -> 0x1B
static char *secondLettersShift="ASDFGHJKL:\"~"; //0x1E -> 0x29
static char *thirdLettersShift="ZXCVBNM<>?";	//0x2C -> 0x35

static char *firstLettersCaps="QWERTYUIOP[]"; //0x10 -> 0x1B
static char *secondLettersCaps="ASDFGHJKL;'`"; //0x1E -> 0x29
static char *thirdLettersCaps="ZXCVBNM,./";	//0x2C -> 0x35

static char *numbersShiftCaps1="!@#$";		//0x20 -> 0xD
static char *numbersShiftCaps2="^&*()_+";		//0x20 -> 0xD
static char *firstLettersShiftCaps="qwertyuiop{}"; //0x10 -> 0x1B
static char *secondLettersShiftCaps="asdfghjkl:\"~"; //0x1E -> 0x29
static char *thirdLettersShiftCaps="zxcvbnm<>?";	//0x2C -> 0x35

void init_keyboard(void)
{
	//enable irq port 1 for keyboard 0x21
	enable_irq(ENABLE_KEYBOARD);
	//dir_open();
	trigger=1;
	index = 0;
	rtcon = 0;
	bufferIndex = 0;
	shiftFlag = 0;	
	capsFlag = -1;
	ctrlFlag = 0;
	enterFlag=0;
	passwordFlag=0;
}

 
 /* keyboard_interrupt
DESC:  Writes characters to the screen by converting scancode to ascii values
INPUT: none
OUTPUT: none
SIDE EFFECTS:Sends characters to be printed to the screen.
*/

void keyboard_interrupt(void)
{
	cli();
	
	uint8_t scancode=0;
	
	scancode=inb(KEYBOARD_DATA_PORT);	//get input from keyboard

	if(scancode==R_CTRL_PRESSED||scancode==L_CTRL_PRESSED)
		ctrlFlag=1;
	if(scancode==R_CTRL_RELEASED||scancode==L_CTRL_RELEASED)
		ctrlFlag=0;

	if(scancode==ALT_PRESSED)
		altFlag=1;
	if(scancode==ALT_RELEASED)
		altFlag=0;

	if(scancode==BACKSPACE_PRESSED)
	{
		if(bufferIndex>0)
		{
			erase();
			bufferIndex--;
			buffer[bufferIndex]='\0';
		}
		send_eoi(1);				//end interrupt request
		sti();
		return;
	}

	if(scancode==ENTER_PRESSED)
	{

		/*uncomment to test functionality of terminal read/write*/
		char buf[BUFFER_MAX + 2];
		enterFlag=1;
		// copy term buffer to buf
		//terminal_read(0, buf, bufferIndex);
		// terminate string
		buf[bufferIndex++]='\n';
		buf[bufferIndex]=buf[BUFFER_MAX+2]='\0';
		enterPressed();
		//printf("%s", "_SPICYOS > ");
		//memset(buffer, 0, BUFFER_MAX);
		//terminal_write(0, buf, sizeof(buf));

 		//use for echoing
		// if(bufferIndex == BUFFER_MAX+1) 
		// 	enterPressed();

		//bufferIndex=0;
		send_eoi(1);				//end interrupt request
		sti();
		return;
	}

	else if(ctrlFlag)
	{
		if(scancode==L_PRESSED)
		{
			//clear screen
			 clear();
			 printf("%s", "_SPICYOS> ");
			 terminal_write(0, buffer, strlen(buffer));
			 send_eoi(1);
			 sti();
			 return;

			
		}
		else if (scancode == TWO_PRESSED) {
			sti();
			send_eoi(1);
			// play_music();
			asm volatile (
				"int $0x80;"
				: /*inputs*/
				: "a"(13), "b"("valgrind")
				: "memory", "cc"
				);
			return;
		}

		else if(scancode==THREE_PRESSED)
		{
			globalTextColor+=1;
			if(globalTextColor>8)
				globalTextColor=0;
			newColor();


		}
		else if(scancode==FOUR_PRESSED)
		{
			globalBackgroundColor+=16;
			if(globalTextColor>16*8)
				globalTextColor=0;
			newColor();
			
		}

		// else if (scancode == ONE_PRESSED) {
		// 	music_flag = 1;
		// 	// cli();
		// 	return;
		// 	cli();
		// 	rtcon=1;

		// 	send_eoi(1);
		// 	sti();
		// 	return;
		// }
		/*
		else if(scancode==ONE_PRESSED)
		{
			//clear screen
			clear();
			uint8_t i;
			int8_t buf[33];

			//display all files, types, and sizes
			for(i=0; i<*(uint32_t*)FILESYS_START; i++)
			{
				//dir_read(&buf);
				dentry_t temp;
				if(read_dentry_by_index(i, &temp)!=0)
				{
					send_eoi(1);
					sti();
					return;
				}

				//print file name
				printf("File Name: ");
				buf[32] = '\0';
				int temp_type = temp.type;
				temp.name[32]='\0';
				printf("%s", temp.name);
				//terminal_write(0,temp.name, sizeof(temp.name));

				//print file type
				printf("  File Type: ");
				printf("%d",temp_type);

				//print file size
				inode_entry_t* inode_entry = (inode_entry_t*)(FILESYS_START + (temp.inode_index + 1) * BLOCK_SIZE);
				printf("  Size: ");
				printf("%d", *(uint32_t*)inode_entry);

				printf("\n");

			}
			send_eoi(1);
			sti();
			return;

		}
		else if(scancode==TWO_PRESSED)
		{
			clear();

			uint8_t buf[33];
			uint8_t rbuf[1024];

			dentry_t temp;
			if(read_dentry_by_name((uint8_t *)"frame0.txt", &temp)!=0)
			{
				send_eoi(1);
				sti();
				return;
			}

			inode_entry_t* inode_entry = (inode_entry_t*)(FILESYS_START + (temp.inode_index + 1) * BLOCK_SIZE);

			uint32_t file_size = *(int32_t*)inode_entry;
			int i;
			for (i = 0; i < file_size; i += sizeof(rbuf)) {
				int len = read_data(temp.inode_index, i, rbuf, sizeof(rbuf));
				int j;

				for (j = 0; j < len; ++j) {
					if (rbuf[j] != '\0') putc(rbuf[j]);
				}			
			}


			//print file nauint32_t inodeme
			printf("\nFile Name: ");
			// int temp_type = temp.type;
			strncpy((int8_t*)(buf), (int8_t*)temp.name, 32);
			buf[32] = '\0';
			terminal_write(0, buf, sizeof(buf));

			//print file type
			printf("  File Type: ");
			printf("%d",temp.type);

			//print file size
			
			printf("  Size: ");
			printf("%d", file_size);

			printf("\n");

			
			send_eoi(1);
			sti();
			return;
			
		}
		else if(scancode==THREE_PRESSED)
		{
			//clear screen
			clear();

			uint8_t buf[33];
			uint8_t rbuf[1024];

			//display all files, types, and sizes

			// check if we reached the end of inodes
			if (index >= *(uint32_t*)FILESYS_START)
				index = 0;

			//dir_read(&buf);
			dentry_t temp;
			if(read_dentry_by_index(index, &temp)!=0)
			{
				send_eoi(1);
				sti();
				return;
			}

			// read_data (temp.inode_index, 0, rbuf, sizeof(rbuf));
			// printf("%s", rbuf);
				// int32_t index;
				// for (index = 0; index < 50000; ++index) {
				// 	putc(rbuf[index]);
				// }

			inode_entry_t* inode_entry = (inode_entry_t*)(FILESYS_START + (temp.inode_index + 1) * BLOCK_SIZE);

			uint32_t file_size = *(uint32_t*)inode_entry;
			int i;
			for (i = 0; i < file_size; i += sizeof(rbuf)) {
				int len = read_data(temp.inode_index, i, rbuf, sizeof(rbuf));
				int j;

				for (j = 0; j < len; ++j) {
					if (rbuf[j] != '\0') putc(rbuf[j]);
				}			
			}


			//print file nauint32_t inodeme
			printf("\nFile Name: ");
			// int temp_type = temp.type;
			strncpy((int8_t*)(buf), (int8_t*)temp.name, 32);
			buf[32] = '\0';
			terminal_write(0, buf, sizeof(buf));

			//print file type
			printf("  File Type: ");
			printf("%d",temp.type);

			//print file size
			
			printf("  Size: ");
			printf("%d", file_size);

			printf("\n");

			index++;

			send_eoi(1);
			sti();
			return;
		}
		else if(scancode==FOUR_PRESSED)
		{
			clear();
			rtcon = 1;
			trigger *= 2; //Frequencies change in powers of two

			//change frequency
			if(trigger > 1 && trigger <= 1024)
			{
				int buf = trigger;
				rtc_write(0, &buf, 4);
			}
			else {
				trigger = 1;
			}
				
			send_eoi(1);
			sti();
			return;
		}
			
		
		else if(scancode==FIVE_PRESSED)
		{
			clear();
			//stop rtc test
			printf("%s", "391OS> ");
			rtcon = 0;
			trigger = 1;
			send_eoi(1);
			sti();
			return;
		}
		*/
	}
	else if(altFlag) {
		if (scancode == F1_PRESSED) {
			send_eoi(1);
			change_terminal(curr_term_disp_num, 0);
			newColor();
			printTerminal(1);
			//sti();
		}
		else if (scancode == F2_PRESSED) {
			send_eoi(1);
			change_terminal(curr_term_disp_num, 1);
			newColor();
			printTerminal(2);
			//sti();
		}
		else if (scancode == F3_PRESSED) {
			send_eoi(1);
			change_terminal(curr_term_disp_num, 2);
			newColor();
			printTerminal(3);
			//sti();
		}
		else if (scancode == W_PRESSED) {
			pagefault("page fault");
		}
	}


	else if(asciiConversion(scancode)==0)	//if asciiConversion does not find appropriate value send eoi to PIC
	{
		send_eoi(1);
		sti();
		return;
	}

	else if(bufferIndex<BUFFER_MAX)
	{
		if(shiftFlag==1&&capsFlag==-1)
			buffer[bufferIndex]=asciiConversionShift(scancode);//send ascii value to buffer
		else if(shiftFlag==0 && capsFlag==1)
			buffer[bufferIndex]=asciiConversionCaps(scancode);
		else if(shiftFlag==1 && capsFlag==1)
			buffer[bufferIndex]=asciiConversionShiftCaps(scancode);
		else
			buffer[bufferIndex]=asciiConversion(scancode);
		

		if(passwordFlag!=1)
			putc(buffer[bufferIndex]);	
		bufferIndex++;
		// if(bufferIndex==NUM_COLS)
		// 	newLine();
	}
	
	
	send_eoi(1);				//end interrupt request
	sti();
	
}


/* asciiConversion
DESC:  converts scancode to equivalent ascii value
INPUT: scancode value
OUTPUT: correct ascii character
SIDE EFFECTS:none
*/
uint8_t asciiConversion (uint8_t cmd)
{
	
	if(cmd>=Q_PRESSED && cmd <=SBRACKET_PRESSED)	//check which scancode cmd refers to and get proper ascii value
		return firstLetters[cmd-Q_PRESSED];
	if(cmd>=A_PRESSED && cmd <=BTICK_PRESSED)
		return secondLetters[cmd-A_PRESSED];
	if(cmd>=Z_PRESSED && cmd <=SLASH_PRESSED)
		return thirdLetters[cmd-Z_PRESSED];
	if(cmd >= ONE_PRESSED && cmd <= EQUAL_PRESSED)
		return numbers[cmd - ONE_PRESSED];
	if(cmd==BSLASH_PRESSED)
		return '\\';
	if(cmd==SPACE_PRESSED)
		return ' ';


	if(cmd==L_SHIFT_PRESSED || cmd ==R_SHIFT_PRESSED)
		shiftFlag=1;
	if(cmd==L_SHIFT_RELEASED || cmd == R_SHIFT_RELEASED)
		shiftFlag=0;
	if(cmd==CAPS_PRESSED)
		capsFlag=-capsFlag;

	
	return 0;

}

uint8_t asciiConversionShift (uint8_t cmd)
{

		if(cmd>=Q_PRESSED && cmd <=SBRACKET_PRESSED)	//check which scancode cmd refers to and get proper ascii value
			return firstLettersShift[cmd-Q_PRESSED];
		if(cmd>=A_PRESSED && cmd <=BTICK_PRESSED)
			return secondLettersShift[cmd-A_PRESSED];
		if(cmd>=Z_PRESSED && cmd <=SLASH_PRESSED)
			return thirdLettersShift[cmd-Z_PRESSED];
		if(cmd >= ONE_PRESSED && cmd <= FOUR_PRESSED)
			return numbersShift1[cmd - ONE_PRESSED];
		if(cmd >= SIX_PRESSED && cmd <= EQUAL_PRESSED)
			return numbersShift2[cmd - SIX_PRESSED];
		if(cmd == FIVE_PRESSED)
			return '%';
		if(cmd==BSLASH_PRESSED)
			return '|';
		if(cmd==SPACE_PRESSED)
			return ' ';
		if(cmd==L_SHIFT_PRESSED || cmd ==R_SHIFT_PRESSED)
			shiftFlag=1;
		if(cmd==L_SHIFT_RELEASED || cmd == R_SHIFT_RELEASED)
			shiftFlag=0;
		if(cmd==CAPS_PRESSED)
			capsFlag=-capsFlag;
		
		
		return 0;
}

uint8_t asciiConversionCaps (uint8_t cmd)
{

		if(cmd>=Q_PRESSED && cmd <=SBRACKET_PRESSED)	//check which scancode cmd refers to and get proper ascii value
			return firstLettersCaps[cmd-Q_PRESSED];
		if(cmd>=A_PRESSED && cmd <=BTICK_PRESSED)
			return secondLettersCaps[cmd-A_PRESSED];
		if(cmd>=Z_PRESSED && cmd <=SLASH_PRESSED)
			return thirdLettersCaps[cmd-Z_PRESSED];
		if(cmd >= ONE_PRESSED && cmd <= EQUAL_PRESSED)
			return numbers[cmd - ONE_PRESSED];
		if(cmd==BSLASH_PRESSED)
			return '\\';
		if(cmd==SPACE_PRESSED)
			return ' ';
		if(cmd==L_SHIFT_PRESSED || cmd ==R_SHIFT_PRESSED)
			shiftFlag=1;
		if(cmd==L_SHIFT_RELEASED || cmd == R_SHIFT_RELEASED)
			shiftFlag=0;
		if(cmd==CAPS_PRESSED)
			capsFlag=-capsFlag;
		

		
		return 0;
}

uint8_t asciiConversionShiftCaps (uint8_t cmd)
{
	if(cmd>=Q_PRESSED && cmd <=SBRACKET_PRESSED)	//check which scancode cmd refers to and get proper ascii value
		return firstLettersShiftCaps[cmd-Q_PRESSED];
	if(cmd>=A_PRESSED && cmd <=BTICK_PRESSED)
		return secondLettersShiftCaps[cmd-A_PRESSED];
	if(cmd>=Z_PRESSED && cmd <=SLASH_PRESSED)
		return thirdLettersShiftCaps[cmd-Z_PRESSED];
	if(cmd >= ONE_PRESSED && cmd <= FOUR_PRESSED)
		return numbersShiftCaps1[cmd - ONE_PRESSED];
	if(cmd >= SIX_PRESSED && cmd <= EQUAL_PRESSED)
		return numbersShiftCaps2[cmd - SIX_PRESSED];
	if(cmd == FIVE_PRESSED)
		return '%';
	if(cmd==BSLASH_PRESSED)
		return '|';

	if(cmd==SPACE_PRESSED)
		return ' ';
	if(cmd==L_SHIFT_PRESSED || cmd ==R_SHIFT_PRESSED)
		shiftFlag=1;
	if(cmd==L_SHIFT_RELEASED || cmd == R_SHIFT_RELEASED)
		shiftFlag=0;
	if(cmd==CAPS_PRESSED)
		capsFlag=-capsFlag;
	
	
	return 0;
}


void userpass(char user[], char pass[])
{
	uint8_t x;
	x=0;
	while(x!=1)
	{
		
		printf("username: ");
		while(enterFlag!=1);
		terminal_read(0, user, bufferIndex);
		bufferIndex=0;
		enterFlag=0;
		printf("password: ");
		passwordFlag=1;
		while(enterFlag!=1);
		terminal_read(0, pass, bufferIndex);
		bufferIndex=0;
		enterFlag=0;
		passwordFlag=0;

		if(strncmp(user, username, 128)==0 && strncmp(pass, password, 128)==0)
		{
			
			x=1;
			clear();
		}
		else
		{
			clear();
			printf("Try again...Hint: What always happens?\n");
		}
	}
}


