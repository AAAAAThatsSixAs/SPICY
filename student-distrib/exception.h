#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "lib.h"
#include "exception.h"

/**
 * exceptions macro to make all exceptions
 * DESCRIPTION: handles exception
 * INPUT: none
 * RETURN VALUE: none
 * SIDE EFFECTS: blue screen freeze
 *
**/

extern int32_t halt (uint8_t status);
#define MAKE_EXCEPTION_FUNCTION(exception_func,error) \
void exception_func() {								\
	printf("%s\n", error);							\
	halt(0);										\
	clear();										\
	pagefault(error);								\
}													\


//enum copypasta_t {"one", "two", "three", "four"};


void pagefault(char* err)
{
	clear();
	uint8_t x, i, posX, posY, midlen;
			x=i=0;
			char * pre="What did you just say about me, you little freshman? I'll have you know I graduated top of my class in ECE, and I've been involved in numerous secret MPs, and I have over 300 confirmed internships. I am trained in trial and error and I'm the top coder in the entire university. You are nothing to me but just another target. I will wipe you the out with precision the likes of which has never been seen before on this Earth, mark my words. You think you can get away with saying that to me over the Internet? Think again. As we speak I am contacting my secret network of spies across in github and your IP is being traced right now so you better prepare for the storm. The storm that brings terror into the eyes of coders all around. The storm known as ";
			char * end=". You're done, kid. Not only am I extensively trained in gdb, but I have access to the entire Monad source code and I will use it to its full extent to wipe your miserable code off the face of the continent. If only you could have known what unholy retribution your little \"clever\" comment was about to bring down upon you, maybe you would have held your tongue. But you couldn't, you didn't, and now you're paying the price. I will throw exceptions all over you and you will drown in them.";

			puts(pre);
			posX=getScreenX();
			posY=getScreenY();
			puts(err);
			puts(end);

			midlen = strlen(err);

			while(1)
			{
				putccolorpos(err[i], x, posX + i, posY);
				if(i++>=midlen) {
					i=0;
					x++;
				}
				if(x>7)
					x=0;
				
			}
}
//EXCEPTION_H
#endif 


