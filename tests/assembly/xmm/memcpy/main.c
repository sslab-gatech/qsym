#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "common.h"

int main( int argc, char* argv[] )
{
    char in[16], out[16];

    read(0, in, sizeof(in));
    memcpy(out, in, sizeof(in));

    if (*(int32_t*)out == 0xdeadbeef)
      good();
    else
      bad();
}
