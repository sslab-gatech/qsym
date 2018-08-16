#include "common.h"
#include <unistd.h>

int main(void) {
    int n, v;
    n = v = 0;
    read(0, &n, sizeof(n));
    __asm__(
        "movsb"
        :
        : "S"(&n), "D"(&v));

    if (v == 0xde)
      good();
    else
      bad();
}
