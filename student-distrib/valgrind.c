#include "valgrind.h"

/* init_valgrind
DESC:  sets up global variables for valgrind
INPUT: none
OUTPUT: none
SIDE EFFECTS: writes to global variables
*/
void init_valgrind() {
	head = NULL;
	curr = NULL;
	track = 0;
	mem_total = 0;
	mem_freed = 0;
	bad_frees = 0;
}

/* setTrackOn
DESC:  tell valgrind to start tracking memory
INPUT: none
OUTPUT: none
SIDE EFFECTS: writes to global variable track
*/
void setTrackOn() {
	track = 1;
}

/* setTrackOff
DESC:  tell valgrind to stop tracking memory
INPUT: none
OUTPUT: none
SIDE EFFECTS: writes to global variable track
*/
void setTrackOff() {
	track = 0;
}


/* vmalloc
DESC:  valgrind's malloc
	   if valgrind is not tracking, the normal malloc is called
INPUT: size    -- size of memory to be allocated
	   file    -- file of allocation
	   func    -- function of allocation
	   linenum -- line of allocation
OUTPUT: none
SIDE EFFECTS: allocates memory
*/
void * vmalloc(uint32_t size, const char* file, const char* func, int linenum) {
	if (!track) return xmalloc(size);

	void * tempptr = xmalloc(size);

	node* n = (node*) xmalloc(sizeof(node));
	n->size = size;
	n->ptr = tempptr;

	n->freed = 0;
	n->line = linenum;
	n->next = NULL;
	strncpy(n->funcname, func, VALGRIND_MAX -1);
	n->funcname[VALGRIND_MAX - 1] = '\0';
	strncpy(n->filename, file, VALGRIND_MAX -1);
	n->filename[VALGRIND_MAX - 1] = '\0';
	if (head == NULL) {
		head = n;
		curr = n;
	}
	else {
		n->next = head;
		head = n;
	}
	mem_total += size;
	return tempptr;

}

/* svfree
DESC:  valgrind's free
	   if valgrind is not tracking, the normal free is called
INPUT: ptr -- memory to be freed
OUTPUT: none
SIDE EFFECTS: frees memory
*/
void vfree(void* ptr) {
	if (!track) {
		xfree(ptr);
		return;
	}

	if (ptr == NULL) {
		bad_frees++;
		return;
	}
	node* op = head;
	while (op != NULL) {

		if (op->freed == 0 && ptr == op->ptr) {

			op->freed = 1;
			mem_freed += op->size;
			xfree(ptr);
			return;
		}
		op = op->next;
		if (op < 0)
			break;
	}
	bad_frees++;
}

/* printresults
DESC:  prints valgrind results
INPUT: none
OUTPUT: none
SIDE EFFECTS: writes to global variable track
*/
void printresults() {
	node* op = head;
	printf("\n");
	printf("FREED:\n");
	while(op != NULL) {
		if (op->freed == 1) {
			// printf("alloc'd %d @ mem loc 0x%x in function %s on line %d in %s\n",
			// 	op->size, op->ptr, op->funcname, op->line, op->filename);
			printf("alloc'd %d @ mem loc 0x%x in function %s\n",
				op->size, op->ptr, op->funcname);
		}
		op = op->next;
	}
	printf("\n");
	op = head;
	printf("UNFREED:\n");
	int unfreed_flag = 0;
	while (op != NULL) {
		if (op->freed == 0) {
			unfreed_flag = 1;
			// printf("alloc'd %d @ mem loc 0x%x in function %s on line %d in %s\n",
			// 	op->size, op->ptr, op->funcname, op->line, op->filename);
			printf("alloc'd %d @ mem loc 0x%x in function %s\n",
				op->size, op->ptr, op->funcname);
		}
		op = op->next;
	}
	if (!unfreed_flag)
		printf("none\n");
	
	printf("\nSUMMARY:\n");
	printf("total alloc'd: %d bytes\n", mem_total);
	printf("total free'd: %d bytes \n", mem_freed);
	printf("bad free(s): %d\n", bad_frees);
	if (mem_total == mem_freed)
		printf("\nall blocks were freed -- no leaks are possible\n");
	else {
		printf("%d bytes lost\n", mem_total - mem_freed);
	}
	destroy();
	printf("\n");
}

/* destroy
DESC:  destructor for valgrind
INPUT: none
OUTPUT: none
SIDE EFFECTS: reinitializes global variables for repeated calls of valgrind
*/
void destroy() {
	node * t = head;
	while (t != NULL) {
		node * temp = t->next;
		xfree(t);
		t = temp;
	}
	init_valgrind();
}
