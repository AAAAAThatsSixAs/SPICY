#ifndef SOUND_H
#define SOUND_H
#include "lib.h"
#include "rtc.h"
#include "noize.h"
//from osdev! http://wiki.osdev.org/PC_Speaker

void play_sound(uint32_t nFrequence);
void nosound();
void play_music();
void waitFor (unsigned int secs);

#endif
