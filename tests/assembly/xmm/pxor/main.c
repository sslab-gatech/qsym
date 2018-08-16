#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h>
#include <unistd.h>
#include "common.h"

int main( int argc, char* argv[] )
{
    char in[16], in2[16], out[16];

    read(0, in, sizeof(in));
    read(0, in2, sizeof(in2));

    __asm__ __volatile__(
       "movdqu %1, %%xmm0\n"
       "movdqu %2, %%xmm1\n"
       "pxor %%xmm1, %%xmm0\n"
       "movdqu %%xmm0, %0\n"
       :"=m"(out)
       :"m"(in), "m"(in2)
       :"xmm0"
    );

    if (*(int32_t*)out == 0xdeadbeef)
      good();
    else
      bad();
}
