#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned int a, b;
  read(0, &a, sizeof(a));

  __asm("decl %0\n"
      : "+m"(a)
      ::);
  if (a == 0xdeadbeef)
    good();
  else
    bad();
}
