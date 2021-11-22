#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define SBUFSIZE 33

int main ()
{
    uint8_t buf[SBUFSIZE];

   if (0 != ece391_getargs (buf, SBUFSIZE)) {
        ece391_fdputs (1, (uint8_t*)"could not read argument\n");
        return 3;
    }

    return  ece391_create(buf);

}

