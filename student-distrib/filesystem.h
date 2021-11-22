//Filesystem.h

#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#ifndef ASM

#include "types.h"
#include "lib.h"
#include "paging.h"
#include "keyboard.h"
#include "rtc.h"
#include "x86_desc.h"
#include "system_calls.h"


#define FILENAME_LENGTH 32
#define	BLOCK_SIZE 		4096
#define DIRECTORY_ENTRIES_NUM 63
#define DENTRY_SIZE		64
#define BLOCK_DATA_ADDR 65*4096
#define MAX_INODE_DATA_INDEX 1023
#define NUM_INODES 64
#define NUM_DATABLOCKS 256


//struct header for boot block
typedef struct  {
	uint32_t num_dir;
	uint32_t num_inodes;
	uint32_t num_datablocks;
	uint8_t reserved[52];
} bootblock_header_t;



//struct for directory entry in boot block
typedef struct {
	uint8_t name[32];
	uint32_t type;
	uint32_t inode_index;
	uint8_t reserved[24];
}dentry_t;



//struct for inode entry
typedef struct {
	uint32_t size;
	uint32_t data[1023];
}inode_entry_t;


//variable to hold beginning address of filesystem
uint8_t* filesystem_start;
bootblock_header_t* bootblockg;

//variable to hold directory name index
uint32_t dir_index;

//array to hold used inodes and datablocks
uint8_t used_inodes[64];
uint8_t used_dbs[256];
uint32_t last_directory;
uint32_t directory_returned;

 	
//function headers
extern int32_t file_open(const uint8_t* filename);
int32_t file_write(int32_t fd,const  void *buf, int32_t nbytes);
int32_t file_close(int32_t fd);
int32_t file_read_sys(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_open(const uint8_t *filenam);
int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t dir_close(int32_t fd);
int32_t dir_read_sys(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_read(int8_t* buf, uint32_t length, uint8_t* fname, uint32_t offset);
int32_t file_read( uint8_t* buf, uint32_t length, uint8_t* fname, uint32_t	offset);

int32_t file_create(const uint8_t* filename);
int32_t file_remove(const uint8_t* filename);
int32_t find_new_db();

//struct to hold


 int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
 int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
 int32_t read_data (uint32_t inode, uint32_t	offset, uint8_t* buf, uint32_t length);

#endif
#endif
