#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  int a, b;
  read(0, &a, sizeof(a));

  __asm__("mov $0x8, %%ecx\n"
      "shrl %%cl, %0\n"
    : "+m"(a)
    ::);

  if (a == (0xdeadbeef >> 8))
    good();
  else
    bad();
}
