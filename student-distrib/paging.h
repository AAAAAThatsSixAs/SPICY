#ifndef PAGING_H
#define PAGING_H

#ifndef ASM

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"

#define KERNEL_LOCATION 0x400000
#define SHELL_LOCATION 0x800000
#define USER_PROGRAM 0x120000
#define VMEM_LOCATION 0x8000000 //128MB


#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_SIZE 4096
#define REMAP 0x400000
#define SIZE_WRITE_PRESENT 0x83
#define SIZE_USER_WRITE_PRESENT 0x87
#define VIDEO_MEMORY_TO_PHYSICAL 0xB8000
#define SMALL_PAGE_SIZE 0x1000
#define WRITE_PRESENT 0x03
#define WRITE 0x02
#define USER_WRITE_PRESENT 0x07
#define V_MEM_PAGE_LOC 0xB8000
#define PAGE33 0x21
#define START_FREE_MEM 0xBA000
#define MALLOC_LOCATION 15

/* maps virtual memory to physical memory */
extern void virt_to_phys(uint32_t phys_addr, uint32_t virt_addr);

/*maps terminal virt mem to virt mem*/
extern void user_virt_to_phys(uint32_t phys_addr, uint32_t virt_addr, uint32_t terminal_num);

/*flushes the TLB*/
extern void flush_tlb(void);
/*unmaps pages from their physical location*/
extern void unmap (uint32_t process);
extern void map_vid_mem_user();
#endif //ASM
#endif // PAGING_H
