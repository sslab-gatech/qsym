#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "common.h"


int test(int n) {
  if (n == 0xdeadbeef)
    return 1;
  else
    return 0;
}

int main(void)
{
  uint64_t n = 0, v = 0;
  read(0, &n, sizeof(n));
  __asm("push %1\n"
      "pop %%" MACHINE_DX "\n"
      "mov %%" MACHINE_DX ", %0\n"
      : "=r"(v)
      : "r"(n)
      : MACHINE_DX);

  if (v == 0xdeadbeef)
    good();
  else
    bad();
}
