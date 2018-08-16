#include "common.h"
#include <unistd.h>

int main() {
  int x, y;
  read(0, &x, sizeof(x));
  read(0, &y, sizeof(y));

  __asm__("xadd %1, %0\n"
      : "+r"(y)
      : "r"(x)
      :);

  if (y == 0xdeadbeef)
    good();
  else
    bad();
}
