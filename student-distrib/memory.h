#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"

void* startptr;
void* endptr;
void* page;
uint32_t used;

char mem[8192];

#define MAX_MALLOC_SIZE 4096
#define SIZEOF_UINT32_T 4
#define HEAP_LOCATION 0xc801000 // 204 MB
#define PHYS_MEM_LOCATION 0xc0000
#define HEAP_SIZE 4096
#define SIZEMASK 0xFFFFF
#define FREE 0x80000000
#define IS_FREE 31


void init_memory();
void* xmalloc(uint32_t size);
void xfree(void* ptr);
void* calloc(uint32_t size, uint32_t n);
void* realloc(void* ptr, uint32_t size);
void* sbrk (uint32_t size);

#endif // MEMORY_H
