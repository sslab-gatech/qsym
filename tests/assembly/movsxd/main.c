#include "common.h"
#include <unistd.h>
#include <stdint.h>

#ifndef __x86_64__
  #error "Only for x86_64"
#endif

int main() {
  int a = 0;
  uint64_t b = 0;
  read(0, &a, sizeof(a));

  __asm__("movsxd %1, %0\n"
      : "=r"(b)
      : "m"(a));

  if (b == 0xffffffffdeadbeef)
    good();
  else
    bad();
}
