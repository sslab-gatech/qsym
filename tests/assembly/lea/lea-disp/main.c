#include "common.h"
#include <unistd.h>

int main(void) {
    int n, v;
    read(0, &n, sizeof(n));
    __asm__("mov %1, %%eax\n"
        "lea 0x10(%%eax), %%ebx\n"
        "mov %%ebx, %0\n"
        : "=r"(v)
        : "r"(n)
        : "eax", "ebx");

    if (v == 0xdeadbeef)
      good();
    else
      bad();
}
