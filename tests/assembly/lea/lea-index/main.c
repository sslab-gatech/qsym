#include "common.h"
#include <unistd.h>

int main(void) {
    int n, v;
    read(0, &n, sizeof(n));
    __asm__("mov %1, %%eax\n"
        "lea (, %%eax, 4), %%ebx\n"
        "mov %%ebx, %0\n"
        : "=r"(v)
        : "r"(n)
        : "eax", "ebx");

    if (v == 0xdeadbeec)
      good();
    else
      bad();
}
