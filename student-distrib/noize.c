#include "noize.h"

// ---
// noize.c
// ---

noize_t * make_noize() {
	// allocate memory
	noize_t * ptr = (noize_t *)malloc(sizeof(noize_t));
	// set values
	ptr->head = NULL;
	ptr->tail = NULL;
	ptr->notes = 0;
	// return pointer
	return ptr;
}

void destroy_noize(noize_t * n) {
	// sanity check
	if (n == NULL) return;

	// free all nodes
	noize_node_t * ptr = n->head;
	while (ptr != NULL) {
		noize_node_t * temp = ptr->next;
		free(ptr);
		ptr = temp;
	}

	// free self
	free(n);
 	outb(inb(0x61) & 0xFC, 0x61);
}

void play_noize(noize_t * x) {
	clear();
	printf("playing music!\n");
	noize_node_t * ptr = x->head;
	while (ptr != NULL) {
		// play
		play_sound(ptr->freq);
		sleep_rtc(ptr->time);
		ptr = ptr->next;
	}
}

void add_sixteenth(noize_t * x, const char * note) {
	add_note(x, TIMEPERSIXTEENTH, note);
}
void add_eigth(noize_t * x, const char * note) {
	add_note(x, 2 * TIMEPERSIXTEENTH, note);
}
void add_quarter(noize_t * x, const char * note) {
	add_note(x, 4 * TIMEPERSIXTEENTH, note);
}
void add_half(noize_t * x, const char * note) {
	add_note(x, 8 * TIMEPERSIXTEENTH, note);
}
void add_whole(noize_t * x, const char * note) {
	add_note(x, 16 * TIMEPERSIXTEENTH, note);
}

void add_note(noize_t * x, uint16_t time, const char * note) {
	noize_node_t * n = create_node(time, note);
	add_note_ptr(x, n);
}

void add_note_freq(noize_t * x, uint16_t time, uint16_t freq) {
	noize_node_t * n = create_node_freq(time, freq);
	add_note_ptr(x, n);
}

void add_note_ptr(noize_t * x, noize_node_t * n) {
	if (x->head == NULL || x->tail == NULL) {
		x->head = n;
		x->tail = n;
		x->notes = 1;
	} else {
		x->tail->next = n;
		n->prev = x->tail;
		x->tail = n;
		x->notes++;
	}
}

noize_node_t * pop_note(noize_t * x) {
	if (x->head == NULL)
		return NULL;
	noize_node_t * temp = x->tail;
	x->tail = temp->prev;
	temp->prev = NULL;
	temp->next = NULL;
	x->tail->next = NULL;

	return temp;
}

noize_node_t * create_node(uint16_t time, const char * note) {
	return create_node_freq(time, note_to_freq(note));
}

noize_node_t * create_node_freq(uint16_t time, uint16_t freq) {
	noize_node_t * x = (noize_node_t *)malloc(sizeof(noize_node_t));
	x->time = time;
	x->freq = freq;
	x->prev = NULL;
	x->next = NULL;
	return x;
}

uint16_t note_to_freq(const char * note) {
	// [A-G]
	// [#b]?
	// [0-8]?
	uint32_t freq = 0;

	// note
	switch (note[0]) {
		case 'A':
			freq = 44000;
			break;
		case 'B':
			freq = 49388;
			break;
		case 'C':
			freq = 26163;
			break;
		case 'D':
			freq = 29366;
			break;
		case 'E':
			freq = 32963;
			break;
		case 'F':
			freq = 34923;
			break;
		case 'G':
			freq = 39200;
			break;
	}

	// sharps/flats?
	if (note[1] == '#' || note[1] == 'b') {
		if ((note[0] == 'C' && note[1] == '#')
			|| (note[0] == 'D' && note[1] == 'b'))
			freq = 27718;
		if ((note[0] == 'D' && note[1] == '#')
			|| (note[0] == 'E' && note[1] == 'b'))
			freq = 31113;
		if ((note[0] == 'F' && note[1] == '#')
			|| (note[0] == 'G' && note[1] == 'b'))
			freq = 36999;
		if ((note[0] == 'G' && note[1] == '#')
			|| (note[0] == 'A' && note[1] == 'b'))
			freq = 41530;
		if ((note[0] == 'A' && note[1] == '#')
			|| (note[0] == 'B' && note[1] == 'b'))
			freq = 46616;
	}

	// octave?
	int octave = 4;
	if (note[1] >= '0' && note[1] <= '8')
		octave = note[1] - '0';
	if (note[2] >= '0' && note[2] <= '8')
		octave = note[2] - '0';
	
	while (octave > 4) {
		freq = freq * 2;
		octave--;
	}
	while (octave < 4) {
		freq = freq / 2;
		octave++;
	}

	return (uint16_t)(freq / 100);
}



/** EXAMPLE
 * noize_t * song = make_noize();
 * // add notes
 * add_half(song, "G3");
 * add_half(song, "E4");
 * add_half(song, "C4");
 * // play song x3
 * play_noize(song);
 * play_noize(song);
 * play_noize(song);
 * // free memory
 * destroy_noize(song);
**/
