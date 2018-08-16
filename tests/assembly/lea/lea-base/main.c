#include "common.h"
#include <unistd.h>

int main(void) {
    int a,b , v;
    read(0, &a, sizeof(a));
    read(0, &b, sizeof(b));

    if (a != 0xbadf00d)
      return 0;

    __asm__("lea (%1, %2), %%eax\n"
        "mov %%eax, %0\n"
        : "=r"(v)
        : "r"(a), "r"(b)
        : "eax");

    if (v == 0xdeadbeef)
      good();
    else
      bad();
}
