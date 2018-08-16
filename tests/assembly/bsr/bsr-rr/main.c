#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  int a = 0;
  read(0, &a, sizeof(a));

  __asm__("bsr %0, %0\n"
      : "+r"(a));

  if (a == 12)
    good();
  else
    bad();
}
