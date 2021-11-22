#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#ifndef ASM


#include "filesystem.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"
#include "sound.h"

#define BUF_SIZE 4
#define MAX_CMD 3 //max number of commands
#define FILENAME_SIZE 32
#define MAGIC_1 0x7F
#define MAGIC_2 0x45
#define MAGIC_3 0x4C
#define MAGIC_4 0x46
#define MAX_NUM_PROCESS 6	
#define KSTACK_SIZE 8192
#define VMEM_OFFSET 0x08048000
#define TWENTY_FOUR 24
#define _4MB 0x400000
#define _128MB 128*1024*1024
#define FDMIN 2
#define FDMAX 8
#define MAX_CMD_LENGTH 64


//setup fileops jumptable
typedef int32_t (*function_ptr_open)(const uint8_t* );	
typedef int32_t (*function_ptr_read)(int32_t, void*, int32_t);
typedef int32_t (*function_ptr_write)(int32_t, const void*, int32_t);
typedef int32_t (*function_ptr_close)(int32_t);

typedef struct {
	function_ptr_open open_ptr;
	function_ptr_read read_ptr;
	function_ptr_write write_ptr;
	function_ptr_close close_ptr;
}fileops_table;

//struct to hold currently open files
typedef struct {
	fileops_table* fileops_tableptr;
	int32_t inode;
	int32_t file_loc;
	int32_t flags;
}file_descriptor_entry;

typedef struct{
	file_descriptor_entry open_files[8];
	uint32_t privilege;
	uint32_t ID;
	uint32_t *parent_ptr;
	uint32_t parent_ID;
	uint32_t saved_esp;
	uint32_t saved_ebp;
	uint8_t args[64];
	uint32_t parent_entry_addr;
}pcb_t;

pcb_t * curr_pcb;

fileops_table rtc_fileops;
fileops_table dir_fileops;
fileops_table file_fileops;
fileops_table stdio_fileops;

//setup RnTC jumptable
int8_t pid_list[6];
int32_t process_num[3];
int32_t isfirst;

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);

int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);
uint32_t init_file_array(void);
int32_t find_new_pid();
void valgrind(void);

/* EXTRA CREDIT REAX ONLy */
int32_t create(const uint8_t* filename);
int32_t remove(const uint8_t* filename);

#endif
#endif
