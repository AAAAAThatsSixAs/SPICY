#ifndef TERMINAL_H
#define TERMINAL_H
#ifndef ASM

#include "keyboard.h"
#include "paging.h"
#define _three 3
#define VIDMEM_TERMINAL_LOC 0x6E00000 //110MB
#define VIDMEM_SIZE 0x2000 //8KB
#define _4KB 0x1000 //4KB

typedef struct {
	uint32_t p_num;
	uint32_t active;
	uint32_t esp;
	uint32_t ebp;
	uint32_t isfirst;
	uint32_t * curr_pcb_ptr;
	char buf[128];
	uint32_t bufindex;
	uint32_t * vmem_ptr;
	uint32_t screen_x, screen_y;
}terminal_t;

terminal_t terminals[_three];
int32_t curr_term_disp_num;

void init_terminal(void);
void start_terminal(int terminal);
void change_terminal(int curr_term_num, int next_term_num);

int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
#endif
