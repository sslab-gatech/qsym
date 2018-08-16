#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  int a = 0;
  int b = 0;
  read(0, &a, sizeof(a));

  __asm__("bsr %1, %0"
      : "=r"(b)
      : "m"(a) );

  if (b == 16)
    good();
  else
    bad();
}
