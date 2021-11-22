#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    int32_t fd, i;
    uint8_t buf[1024];

    if (0 != ece391_getargs (buf, 1024)) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	return 3;
    }

    if (-1 == (fd = ece391_open (buf))) {
        ece391_fdputs (1, (uint8_t*)"file not found\n");
	return 2;
    }

    uint8_t stuff[5000];
    for(i = 0; i < 5000; i++)
        stuff[i] = 'a';

    ece391_write(fd, stuff, 5000);

    return 0;
}