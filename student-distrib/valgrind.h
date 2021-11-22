#ifndef VALGRIND_H
#define VALGRIND_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "memory.h"

#define VALGRIND_MAX 40

/* valgrind macros
DESC:  changes all instances of malloc and free to vmalloc and vfree
	   the user won't notice the difference 
	   this allows valgrind to track memory without being noticeable
INPUT: none
OUTPUT: none
SIDE EFFECTS: changes functions
*/

#define malloc(size) vmalloc(size, __FILE__, __FUNCTION__, __LINE__)
#define free(size) vfree(size)

typedef struct {
	uint32_t size;
	void* ptr;
	uint32_t freed;
	int line;
	char filename[VALGRIND_MAX];
	char funcname[VALGRIND_MAX];
	void* next;
} node;

node* head;
node* curr;

int track;

void * vmalloc(uint32_t size, const char* file, const char* func, int linenum);
void vfree(void* ptr);
void printresults();
void init_valgrind();
void destroy();

void setTrackOn();
void setTrackOff();

uint32_t mem_total;
uint32_t mem_freed;
uint32_t bad_frees;

#endif // VALGRIND_H
