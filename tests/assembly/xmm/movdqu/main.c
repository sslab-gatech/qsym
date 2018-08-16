#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h>
#include <unistd.h>
#include "common.h"

int main( int argc, char* argv[] )
{
    char in[16], out[16];

    read(0, in, sizeof(in));

    __asm__ __volatile__(
       "movdqu %1, %%xmm0\n\t"
       "movdqu %%xmm0, %0"
       :"=m"(out)
       :"m"(in)
       :"xmm0"
    );

    if (*(int32_t*)out == 0xdeadbeef)
      good();
    else
      bad();
}
