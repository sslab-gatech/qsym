#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  int n, v;
  uintptr_t tmp;
  read(0, &n, sizeof(n));

  // TODO: Support x86-64
#ifdef __i386__
  __asm__(
      "push %2\n"
      "pop %1\n"
      "mov %1, %0\n"
      : "=r"(v), "+r"(tmp)
      : "m"(n)
      : "eax");
#else
  v = n;
#endif
  if (v == 0xdeadbeef)
    good();
  else
    bad();
}
