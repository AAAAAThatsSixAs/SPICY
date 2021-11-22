#include "memory.h"
#include "types.h"
#include "paging.h"
#include "system_calls.h"


/* init_memory
DESC:  sets up heap and all global variables to keep track of memory
INPUT: none
OUTPUT: none
SIDE EFFECTS: adds a page and writes to global vars
*/
void init_memory() {
	virt_to_phys(PHYS_MEM_LOCATION, HEAP_LOCATION);
	page = (void*)HEAP_LOCATION;
	startptr = endptr = page;
	used = 0;
}

/* xmalloc
DESC:  standard malloc function
	   steps through heap to find location for memory
INPUT: none
OUTPUT: pointer to allocated memory
SIDE EFFECTS: writes to heap
*/
void* xmalloc(uint32_t size) {
	//sanity check
	if (size > MAX_MALLOC_SIZE) 
		return NULL;

	//check if available memory
	void* ptr = startptr;
	while (ptr < endptr) {
		//check size and status
		uint32_t isize = (*(uint32_t *)ptr) & SIZEMASK;
		uint32_t isfree = (*(uint32_t *)ptr) >> IS_FREE; 

		if (isfree && isize == size) {
			*(uint32_t*)ptr = *(uint32_t*)(ptr + size + SIZEOF_UINT32_T) = (*(uint32_t *)ptr) & SIZEMASK;
			
			return ptr + SIZEOF_UINT32_T;
		}	
		else if (isfree && (isize >= size + 2 * SIZEOF_UINT32_T)) {
			//split
			*(uint32_t*)ptr = *(uint32_t*)(ptr + size + SIZEOF_UINT32_T) = size & SIZEMASK;
			//set rest of data as free
			*(uint32_t*)(ptr + size + 2 * SIZEOF_UINT32_T) = *(uint32_t*)(ptr + isize + SIZEOF_UINT32_T) = (isize - size - 2 * SIZEOF_UINT32_T) & SIZEMASK;
			
			return ptr + SIZEOF_UINT32_T;
		}
		//go next
		ptr = ptr + isize + SIZEOF_UINT32_T;
	}

	//sbrk
	void* head = sbrk(size + SIZEOF_UINT32_T);
	if (head == (void*)-1) 
		return NULL;
	*(uint32_t*)head = *(uint32_t*)(head + size + SIZEOF_UINT32_T) = size & SIZEMASK;
	endptr += size;
	
	return head + SIZEOF_UINT32_T;
}

/* xfree
DESC:  standard free
	   finds location of pointer and frees it
	   coalesces data together in the case of multiple frees
INPUT: void* ptr -- a pointer to the memory to be freed
OUTPUT: none
SIDE EFFECTS: writes to heap
*/
void xfree(void* ptr) {
	//sanity check
	if (ptr == NULL 
	 || ptr > endptr
	 || ptr < startptr)
		return;
	//set free
	void* rptr = ptr - SIZEOF_UINT32_T;
	uint32_t size = (*(uint32_t*)ptr) & SIZEMASK;

	uint32_t cup = 0;
	uint32_t cdown = 0;
	void * cstart = rptr;
	uint32_t nsize = size; 
	//coalesce up
	if ((rptr - SIZEOF_UINT32_T > startptr) && (*(uint32_t*)(rptr - SIZEOF_UINT32_T) >> IS_FREE)) {
		uint32_t xsize = *(uint32_t*)(rptr - SIZEOF_UINT32_T) & SIZEMASK;
		if (rptr - 2*SIZEOF_UINT32_T - xsize >= startptr) {
			// ensure mem is after the start
			cup = 1;
			cstart = rptr - 2*SIZEOF_UINT32_T - xsize;
			nsize += xsize + 2*SIZEOF_UINT32_T;
		}
	}

	//coalesce down
	if ((rptr + size + 2*SIZEOF_UINT32_T < endptr) && *(uint32_t*)(rptr + size + 2 * SIZEOF_UINT32_T) >> IS_FREE) {
		cdown = 1;
		uint32_t xsize = *(uint32_t*)(rptr + size + 2 * SIZEOF_UINT32_T) & SIZEMASK;
		nsize += xsize + 2*SIZEOF_UINT32_T;
	}

	*(uint32_t*)cstart = *(uint32_t*)(cstart + nsize + SIZEOF_UINT32_T) = FREE | (nsize & SIZEMASK);
}

/* calloc
DESC:  allocates memory and sets it to 0
	   done with memset
INPUT: size -- size of memory
	   n    -- number of times that memory should be allocated
OUTPUT: none
SIDE EFFECTS: writes to heap
*/
void* calloc(uint32_t size, uint32_t n) {
	void* ptr = (void*) xmalloc(size * n);
	if (ptr == NULL) 
		return NULL;
	//set memory to 0
	memset(ptr, 0, size * n); 
	return ptr;
}

/* realloc
DESC:  takes allocated memory and reallocates it
	   done with memcpy
INPUT: ptr -- pointer to allocated memory
       size -- size of memory wanted
OUTPUT: none
SIDE EFFECTS: writes to heap
*/
void* realloc(void* ptr, uint32_t size) {
	void* rptr = ptr - SIZEOF_UINT32_T;
	uint32_t isize = *(uint32_t*)rptr & SIZEMASK;
	if (size <= isize) 
		return ptr;
	void* nptr = xmalloc(size);
	if (nptr == NULL) return NULL;
	//copy memory
	memcpy(nptr, ptr, isize);
	xfree(ptr);
	return nptr;
}

/* sbrk
DESC:  system's way to keep track of memory
INPUT: size of memory to be allocated
OUTPUT: location of memory on success
		-1 on failure
SIDE EFFECTS: writes to global variables
*/
void* sbrk (uint32_t size) {
	//sanity check
	if (size <= 0)
		return (void*) -1;
	used += size;
	endptr += size;

	return page + used;
}
