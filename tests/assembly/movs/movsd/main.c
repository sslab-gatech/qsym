#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  int64_t a, b, c = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  __asm__(
      "movsd"
      :
      : "S"(&a), "D"(&c));

  if (b == c)
    good();
  else
    bad();
}
