#include "sound.h"
//Play sound using built in speaker
void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb( 0xb6,0x43);
 	outb((uint8_t) (Div),0x42 );
 	outb((uint8_t) (Div >> 8),0x42);
 
        //And play the sound using the PC speaker
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(tmp | 3, 0x61);
 	}
 }
 
 //make it shutup
void nosound() {
outb(inb(0x61) & 0xFC, 0x61);
 }
 
 //Make a beep
 void play_music() {
	 	noize_t* song = make_noize();
		add_eigth(song, "A3");
		add_eigth(song, "C4");
		add_eigth(song, "E4");
		add_quarter(song, "F4");
		add_eigth(song, "E4");
		add_quarter(song, "F4");
		add_eigth(song, "E4");
		add_quarter(song, "F4");
		add_eigth(song, "E4");
		add_half(song, "A2");
		add_eigth(song, "A2");
		add_quarter(song, "A4");
		add_sixteenth(song, "A4");
		add_quarter(song, "E5");
		add_sixteenth(song, "E5");
		add_quarter(song, "D5");
		add_sixteenth(song, "D5");
		add_quarter(song, "E5");
		add_sixteenth(song, "E5");
		add_quarter(song, "G5");
		add_sixteenth(song, "G5");
		add_quarter(song, "E5");
		add_sixteenth(song, "E5");

		play_noize(song);
		destroy_noize(song);

 	// uint32_t* ptr = (uint32_t*)malloc(sizeof(uint32_t) * 10);
}
