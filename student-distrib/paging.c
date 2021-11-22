#include "paging.h"
/**
 * paging_init()
 * DESCRIPTION: Initalizes the page directory
 * 				Creates space for video memory and the kernel 
 * INPUT: NONE
 * RETURN VALUE: NONE
 * SIDE EFFECTS: Writes to page_directory and video_memory global buffers
**/


static uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static uint32_t video_memory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static uint32_t video_memory_2[PAGE_DIRECTORY_SIZE] __attribute__((aligned(PAGE_SIZE)));

void init_paging() 
{
	int i;
	for (i = 0; i < PAGE_DIRECTORY_SIZE; ++i) {
		page_directory[i] = WRITE;
		video_memory[i] = (i * SMALL_PAGE_SIZE) | WRITE; 		//initialize physical and video memory
	}


	page_directory[0] = ((unsigned int)video_memory | WRITE_PRESENT);
	page_directory[1] = KERNEL_LOCATION | SIZE_WRITE_PRESENT;  		//turn on size, write, present bits

	video_memory[ (int) VIDEO_MEMORY_TO_PHYSICAL / SMALL_PAGE_SIZE ] |= WRITE_PRESENT;

	/* set cr0, cr3, cr4 for paging */
	asm volatile ("			    \n\
			mov %0, %%eax		\n\
			mov %%eax, %%cr3"
            : // no outputs 
            : "r" (page_directory)	//input
            : "eax","memory", "cc"	//clobber eax
        );

	asm volatile ("                   \n\
			mov %%cr4, %%eax		  \n\
			or $0x10, %%eax    	      \n\
			mov %%eax, %%cr4"						
            : 
            : 
            : "eax", "memory", "cc" //clobber eax
        );

	asm volatile ("                   \n\
			mov %%cr0, %%eax		  \n\
			or $0x80000000, %%eax     \n\
			mov %%eax, %%cr0"						
            : 
            : 
            : "eax", "memory", "cc" //clobber eax
        );


	printf("Paging has been enabled\n");

	return;
}

void user_virt_to_phys(uint32_t phys_addr, uint32_t virt_addr, uint32_t terminal_num) {
    uint32_t page_loc = virt_addr/REMAP;
    page_directory[page_loc] = phys_addr | SIZE_USER_WRITE_PRESENT;
	video_memory_2[terminal_num] = (V_MEM_PAGE_LOC + (terminal_num * _4KB)) | USER_WRITE_PRESENT;
}

void virt_to_phys(uint32_t phys_addr, uint32_t virt_addr) {
    uint32_t page_loc = virt_addr/REMAP;
    page_directory[page_loc] = phys_addr | SIZE_USER_WRITE_PRESENT;
}
 
void unmap (uint32_t process) {
	if (process <= 1) return;
	page_directory[process + 1] = 0;		//deallocate page
}


void flush_tlb() {
	asm volatile ("                   \n\
		mov %%cr3, %%eax		  \n\
		mov %%eax, %%cr3"						
        : 
        : 
        : "eax", "memory", "cc" //clobber eax
    );
    return;
}

void map_vid_mem_user(){
	page_directory[VMEM_LOCATION/REMAP] = (uint32_t)video_memory_2 | USER_WRITE_PRESENT;
	video_memory_2[4] = V_MEM_PAGE_LOC | USER_WRITE_PRESENT;
	// this was in jd's ec branch ...
	// page_directory[PAGE33] = (uint32_t)video_memory_2 | USER_WRITE_PRESENT;
	// video_memory_2[0] = V_MEM_PAGE_LOC | USER_WRITE_PRESENT;
}

void map_malloc() {
	page_directory[MALLOC_LOCATION] = (uint32_t)video_memory | USER_WRITE_PRESENT;
	video_memory[4] = V_MEM_PAGE_LOC | USER_WRITE_PRESENT;
}
