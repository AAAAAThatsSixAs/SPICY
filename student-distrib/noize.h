#ifndef NOIZE_H
#define NOIZE_H


#include "types.h"
#include "memory.h"
#include "sound.h"
#include "valgrind.h"
#include "lib.h"

#define TIMEPERSIXTEENTH 100

typedef struct {
	uint16_t time;
	uint16_t freq;
	void * prev;
	void * next;
} noize_node_t;

typedef struct {
	noize_node_t * head;
	noize_node_t * tail;
	uint16_t notes;
} noize_t;

// general create/destroy
noize_t * make_noize();
void destroy_noize(noize_t * n);
void play_noize(noize_t * x);

// add notes
void add_sixteenth(noize_t * x, const char * note);
void add_eigth(noize_t * x, const char * note);
void add_quarter(noize_t * x, const char * note);
void add_half(noize_t * x, const char * note);
void add_whole(noize_t * x, const char * note);

void add_note(noize_t * x, uint16_t time, const char * note);
void add_note_freq(noize_t * x, uint16_t time, uint16_t freq);
void add_note_ptr(noize_t * x, noize_node_t * n);
noize_node_t * pop_note(noize_t * x);

// node creation
noize_node_t * create_node(uint16_t time, const char * note);
noize_node_t * create_node_freq(uint16_t time, uint16_t freq);
uint16_t note_to_freq(const char * note);

#endif // NOIZE_H
