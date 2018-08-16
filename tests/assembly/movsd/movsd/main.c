#include "common.h"
#include <unistd.h>

int main(void) {
    int n, v;
    read(0, &n, sizeof(n));
    __asm__(
        "movsd"
        :
        : "S"(&n), "D"(&v));

    if (v == 0xdeadbeef)
      good();
    else
      bad();
}
