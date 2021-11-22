#include "filesystem.h"

/* file_open
DESC:  opens a file
INPUT: none
OUTPUT: none
SIDE EFFECTS:none
*/
int32_t file_open(const uint8_t* filename)
{
	filesystem_start = (uint8_t*)FILESYS_START;
	return 0;
}

/* file_write
DESC:  writes to a file
INPUT: none
OUTPUT: none
SIDE EFFECTS: returns n bytes read, or -1 on failure
*/
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes)
{
	int inodenum = curr_pcb->open_files[fd].inode;
	inode_entry_t* node = (inode_entry_t*)(FILESYS_START + (inodenum + 1) * BLOCK_SIZE);

	int data_i = node->size/BLOCK_SIZE;
	int offset = node->size%BLOCK_SIZE;

	int counter = 0;

	while(counter != nbytes)
	{
		//get a new block if necessary
		if (offset == BLOCK_SIZE)
		{
			offset = 0;
			//find next datablock
			data_i++;
			int newdb = find_new_db();
			if (newdb == -1)
				return counter;
			printf("write: data[%d] now contains new data block # %d \n",data_i, newdb);

			node->data[data_i] = newdb;
			used_dbs[newdb] = 1;
		}

		//write data
		uint8_t* to_write = (uint8_t*)(FILESYS_START + BLOCK_DATA_ADDR + node->data[data_i] * BLOCK_SIZE + offset);
		*to_write = *(uint8_t*)(buf + counter);
		offset++;
		counter++;

	}

	node->size += counter;

	printf("write: %d characters written to file \n", counter);

	return counter;

}

int32_t find_new_db()
{

	int i;
	for(i = 0; i <= NUM_DATABLOCKS; i++)
	{
		if (i == NUM_DATABLOCKS)
		{
			printf("no more datablocks! :(\n");
			return -1;
		}
		if(used_dbs[i] == 0)
			return i;
	}

	return -1;
}

/* file_close
DESC:  closes a file
INPUT: none
OUTPUT: none
SIDE EFFECTS:none
*/
int32_t file_close(int32_t fd)
{
	return 0;
}

/* file_create
DESC:  creates a file
INPUT: none
OUTPUT: none
SIDE EFFECTS:none
*/
int32_t file_create(const uint8_t* filename)
{
	int i = 0;
	while(1)
	{
		if (*(filename + i) == '\0')
		{
			printf("please enter a filename\n");
			return 0;
		}
		if (*(filename + i) != ' ')
			break;
		i++;
	}
	//check if file exists
	dentry_t temp;
	if (read_dentry_by_name(filename, &temp) != -1)
	{
		printf("file already exists, doofus!! xd \n");	
		return 0;
	}

	//find next directory entry
	bootblock_header_t* bb = (bootblock_header_t*)FILESYS_START;

	//update bootblock
	bb->num_dir++;
	bb->num_datablocks++;

	//find new inode, datablock
	int new_inode;
	int new_db;

	for(i = 0; i <= NUM_INODES; i++)
	{
		if (i == NUM_INODES)
		{
			printf("no inodes left! :(\n");
			return 0;
		}
		if(used_inodes[i] == 0)
		{
			new_inode = i;
			break;
		}
	}

	for(i = 0; i <= NUM_DATABLOCKS; i++)
	{
		if (i == NUM_DATABLOCKS)
		{
			printf("no more datablocks! :(\n");
			return 0;
		}

		if(used_dbs[i] == 0)
		{
			new_db = i;
			break;
		}

	}

	//location of new dentry
	dentry_t* new_de = (dentry_t*)(FILESYS_START + bb->num_dir * DENTRY_SIZE);
	for(i = 0; i < FILENAME_LENGTH; i++)
	{
		new_de->name[i] = filename[i];
		if(filename[i] == '\0')
			break;
	}
	new_de->type = 2;
	new_de->inode_index = new_inode;

	//location of new inode
	inode_entry_t* inode_entry_new = (inode_entry_t*)(FILESYS_START + (new_inode + 1) * BLOCK_SIZE);
	inode_entry_new->size = 0;
	inode_entry_new->data[0] = new_db;

	printf("created new file: ");
	for(i = 0; i < FILENAME_LENGTH; i++)
	{
		if (filename[i] == '\0')
			break;
		putc(filename[i]);
	}
	printf(", located at inode #: %d, datablock #: %d\n", new_inode, new_db);


	return 0;

}

/* file_remove
DESC:  removes a file
INPUT: filename - filename
OUTPUT: returns -1 on failure or 0 on success
SIDE EFFECTS:
*/
int32_t file_remove(const uint8_t* filename)
{
	if (filename == NULL)
		return -1;
		int i;

	dentry_t temp;
	if (read_dentry_by_name(filename, &temp) == -1)
	{
		printf("file doesn't exist, doofus!! xd \n");	
		return 0;
	}
	
	if (directory_returned == 0)
	{
		printf("cannot remove . !!!\n");
		return 0;
	}
	

	//printf("inode number: %d\n", temp.inode_index);

	//remove data blocks
	inode_entry_t* inode_entry_rm= (inode_entry_t*)(FILESYS_START + (temp.inode_index + 1) * BLOCK_SIZE);
	
	for(i = 0; i < (inode_entry_rm->size/BLOCK_SIZE) + 1; i++ )
	{
		used_dbs[inode_entry_rm->data[i]] = 0;
		//inode_entry_rm->data[i] = 0;
	}
	inode_entry_rm->size = 0;

	//remove inode entry
	used_inodes[temp.inode_index] = 0;

	bootblock_header_t* bb = (bootblock_header_t*)FILESYS_START;

	//remove directory entry and shift others up
	for (i = 0; i < bb->num_dir - directory_returned; i++)
	{
		//shift blocks up
		dentry_t* curr_d = (dentry_t*)(FILESYS_START + (directory_returned + 1 + i) * DENTRY_SIZE);
		dentry_t* next_d = (dentry_t*)(FILESYS_START + (directory_returned + 1 + i + 1) * DENTRY_SIZE);

		curr_d->type = next_d->type;
		curr_d->inode_index = next_d->inode_index;

		int q;
		for(q = 0; q < FILENAME_LENGTH; q++)
			curr_d->name[q] = next_d->name[q];

	}

	bb->num_dir--;
	
	printf("rm: removed %s \n", filename);

	return 0;
}



/* file_read_sys
DESC:  reads a file
INPUT: fd - file descriptor
		buf - buffer
		nbytes - bytes to read
OUTPUT: returns -1 on failure or # of bytess read
SIDE EFFECTS:
*/
int32_t file_read_sys(int32_t fd, void* buf, int32_t nbytes)
{
	int ret;

	//check for valid arguments
	if (nbytes < 0 || buf == 0)
		return -1;

	//read data given inode
	ret = read_data(curr_pcb->open_files[fd].inode, curr_pcb->open_files[fd].file_loc, (uint8_t*)buf, nbytes);

	if (ret != -1)
		(curr_pcb->open_files[fd]).file_loc += ret;

	return ret;


}


/* file_read
***TESTING FUNCTING FOR MP3.2 - find file by name*****
DESC:  reads a file
INPUT: fname - name of file
		offset - offset of bits to begin
		buf - buffer to store data
		length - length of bytes to read
OUTPUT: returns -1 on failure or # of bytess read
SIDE EFFECTS:
*/
int32_t file_read( uint8_t* buf, uint32_t length, uint8_t* fname, uint32_t	offset)
{

	int ret;

	//check for valid arguments
	if (fname == 0 || offset < 0 || buf == 0 || length < 0)
		return -1;

	//load dentry struct
	dentry_t de;

	if (read_dentry_by_name(fname, &de) != 0)
		return -1;

	//read data given inode
	ret = read_data(de.inode_index, offset, buf, length);

	return ret;
}

/* dir_open
DESC:  open directory
INPUT: none
OUTPUT: none
SIDE EFFECTS: returns -1 because you cant write to dir
*/
int32_t dir_open(const uint8_t *filename)
{
	filesystem_start = (uint8_t*)FILESYS_START;
	dir_index = 0;
	return 0;
}

/* dir_write
DESC:  writes to a dir
INPUT: none
OUTPUT: none
SIDE EFFECTS: returns -1 because you cant write to dir
*/
int32_t dir_write(int32_t fd, void const *buf, int32_t nbytes)
{
	return -1;
}

/* dir_close
DESC:  closes dir
INPUT: none
OUTPUT: none
SIDE EFFECTS: returns -1 because you cant write to dir
*/
int32_t dir_close(int32_t fd)
{
	return 0;
}


/* dir_read_sys
DESC:  reads a dir wrapper
INPUT: buf - buffer to store data
OUTPUT: returns 0 on success
SIDE EFFECTS:
*/
int32_t dir_read_sys(int32_t fd, void* buf, int32_t nbytes)
{
	//cast struct pointers to start of filesystem and bootblock
	filesystem_start = (uint8_t*)FILESYS_START;
	bootblock_header_t * bootblock = (bootblock_header_t*)FILESYS_START;
	bootblockg = bootblock;

	//check if directory index should loop back around
	if (dir_index >= *(uint32_t*)FILESYS_START)
		return 0;

	//get pointer to directory name
	int8_t* dirname = (int8_t*)(FILESYS_START + DENTRY_SIZE + dir_index * DENTRY_SIZE);

	//copy directory name to buffer
	strncpy(buf, dirname, FILENAME_LENGTH);

	dir_index++;

	int32_t retval = strlen(dirname);
	if (retval >= FILENAME_LENGTH)
		retval = FILENAME_LENGTH;

	return retval;

}
/* dir_read
DESC:  reads a dir
INPUT: buf - buffer to store data
OUTPUT: returns 0 on success
SIDE EFFECTS:
*/
int32_t dir_read(int8_t* buf, uint32_t length, uint8_t* fname, uint32_t offset)
{
	//cast struct pointers to start of filesystem and bootblock
	filesystem_start = (uint8_t*)FILESYS_START;
	bootblock_header_t * bootblock = (bootblock_header_t*)FILESYS_START;
	bootblockg = bootblock;

	//check if directory index should loop back around
	if (dir_index >= *(uint32_t*)FILESYS_START)
		dir_index = 0;

	//get pointer to directory name
	int8_t* dirname = (int8_t*)(FILESYS_START + DENTRY_SIZE + dir_index * DENTRY_SIZE);

	//copy directory name to buffer
	strncpy(buf, dirname, FILENAME_LENGTH);

	dir_index++;

	return 0;
}

/* read_dentry_by_name
DESC:  populates a dentry struct with dentry of given file name
INPUT: fname - name of file
		dentry - dentry struct to hold values
OUTPUT: returns 0 on success, -1 on failure
SIDE EFFECTS:
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{


	//cast struct pointers to start of filesystem and bootblock
	filesystem_start = (uint8_t*)FILESYS_START;

 	//set dentry_index to first directory entry start (skips bootblock)
 	uint8_t* dentry_index = (uint8_t*)(filesystem_start + DENTRY_SIZE);

 	//variable for directory name to compare
 	int8_t dname[FILENAME_LENGTH + 1];


	//error checking
	//if (strlen((int8_t*)fname) > FILENAME_LENGTH + 1)
		//return -1;

	if (*fname == '\0') 
		return -1;

	int8_t fname_check[FILENAME_LENGTH + 1];
	strncpy(fname_check, (int8_t*)fname, FILENAME_LENGTH);
	fname_check[FILENAME_LENGTH] = '\0';


 	int i = 0;
 	for (i = 0; i < DIRECTORY_ENTRIES_NUM; i++)
 	{
 		//copy name to dname buffer
 		strncpy(dname, (int8_t*)dentry_index, FILENAME_LENGTH);
 		//need to include a terminal EOS just in case the filename is missing one
 		dname[FILENAME_LENGTH] = '\0';

 		//check if string lengths are the same
 		if (strlen((int8_t*) dname) == strlen((int8_t*)fname_check))
 		{
 			//check if string values are the same
 			if (strncmp((int8_t*)dname, (int8_t*)fname_check, strlen((int8_t*) dname)) == 0)
 			{

 				//copy directory information into dentry
 				strncpy((int8_t*)(dentry->name), (int8_t*)dentry_index, FILENAME_LENGTH);
 					
 				//cast dentry_index to dentry struct
 				dentry_t* dentry_to_copy = (dentry_t*)dentry_index;
 				
 				//strncpy((int8_t*)dname,(int8_t*)dentry_index, FILENAME_LENGTH);
 				//printf(dname);
 				dentry->type = dentry_to_copy->type;
 				
 				dentry->inode_index = dentry_to_copy->inode_index;

 				directory_returned = i;

 				return 0;
 			}
 		}

 		//move on to next directory
 		dentry_index += DENTRY_SIZE;
 	}

 	return -1;

}


/* read_dentry_by_index
DESC:  populates a dentry struct with dentry of given index
INPUT: index- index of file
		dentry - dentry struct to hold values
OUTPUT: returns 0 on success, -1 on failure
SIDE EFFECTS:
*/int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{

	filesystem_start = (uint8_t*)FILESYS_START;	

	//check if index is valid
	if (index < 0 || index >= DIRECTORY_ENTRIES_NUM)
		return -1;

	//set dentry_index to index of filesystem * index * offset
	dentry_t* dentry_index = (dentry_t*)(filesystem_start + index * DENTRY_SIZE + DENTRY_SIZE);

	//copy dentry values
	//dentry_t* dentry_to_copy = (dentry_t*)dentry_index;
	strncpy((int8_t*)dentry,(int8_t*)dentry_index, FILENAME_LENGTH);
	dentry->type = dentry_index->type;
	dentry->inode_index = dentry_index->inode_index;

	return 0;


}



/* read_data
DESC:  reads data and writes to a buffer
INPUT: inode - inode number
		offset - offset of bytess to begin read
		buf - buffer to store data
		length - length of bytes to read
OUTPUT: returns number of bytes read on success, -1 on failure
SIDE EFFECTS: none
*/
int32_t read_data (uint32_t inode, uint32_t	offset, uint8_t* buf, uint32_t length)
{

	//cast struct pointers to start of filesystem and bootblock
	filesystem_start = (uint8_t*)FILESYS_START;
	bootblock_header_t* bootblock = (bootblock_header_t*)FILESYS_START;
	inode_entry_t* inode_entry = (inode_entry_t*)(FILESYS_START + (inode + 1) * BLOCK_SIZE);

	if (strlen((int8_t*)inode_entry) == FILENAME_LENGTH)
		return -1;

	//error checking
	if (inode < 0 )
		return -1;

	if (length < 0 || offset < 0)
		return -1;

	if (offset > inode_entry->size)
		return 0;

	//variables to store current block, data offset, and bytes read
	uint32_t curr_datablock = offset / BLOCK_SIZE;
	uint32_t data_offset = offset % BLOCK_SIZE;	
	uint32_t bytes_read = 0;

	//pointer to location of first data byte
	uint8_t* data_loc = (uint8_t*)(FILESYS_START + BLOCK_DATA_ADDR + inode_entry->data[curr_datablock] * BLOCK_SIZE + data_offset);


	//printf("numinodes: %d, firstblock: %d, data_loc: %x, *data_loc: %c", bootblock->num_inodes, inode_entry->data[curr_datablock], data_loc, *(char*)data_loc);

	//read bytes
	while (bytes_read < length)
	{
	
		//store in buffer
		buf[bytes_read] = *data_loc;
		//putc(buf[bytes_read]);

		//increment variables
		bytes_read++;
		data_loc++;
		data_offset++;

		//check end of file
		if (offset + bytes_read >= inode_entry->size)
			return bytes_read;

		//check if end of block
		if (data_offset % BLOCK_SIZE == 0)
		{
			if (curr_datablock + 1 > MAX_INODE_DATA_INDEX)
				return bytes_read;
			if (inode_entry->data[curr_datablock + 1] >= bootblock->num_datablocks)
				return -1;

			//find new block and set new data location
			data_offset = 0;
			curr_datablock++;

			//printf("new block at: %d\n", inode_entry->data[curr_datablock]);

			data_loc = (uint8_t*)(FILESYS_START + BLOCK_DATA_ADDR + inode_entry->data[curr_datablock] * BLOCK_SIZE);
			// printf("jump 2\n");
		}

	}

	return bytes_read;


}

