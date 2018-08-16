#include "common.h"
#include <unistd.h>

int main() {
  unsigned int a, b;
  read(0, &a, sizeof(a));

  __asm("bswap %1\n"
      "mov %1, %0\n"
      : "=m"(b)
      : "r"(a));

  if (b == 0xdeadbeef)
    good();
  else
    bad();
}
