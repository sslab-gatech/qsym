#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"

uint64_t additive(uint8_t* data, uint32_t s) {
    uint32_t i;
    uint64_t csum = 0xDEADBEEFC0FFEE;

    if(s % sizeof(uint64_t))
        return csum;

    for(i = 0; i < s; i+=sizeof(uint64_t))
        csum += *(uint64_t*)(data+i);
    return csum;
}



int main(void) {
  char data[128];
  read(0, data, sizeof(data));
  uint64_t llresult = additive(data, sizeof(data));
  if (llresult == 0x4242424242424242)
    good();
  else
    bad();
}
