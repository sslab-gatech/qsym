#include "common.h"
#include <unistd.h>
#include <stdint.h>

#ifndef __x86_64__
  #error "Only for x86_64"
#endif

int main() {
  int64_t a, b, c = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  __asm__(
      "movsq"
      :
      : "S"(&a), "D"(&c));

  if (b == c)
    good();
  else
    bad();
}
