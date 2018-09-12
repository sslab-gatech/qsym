#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int *asm_fn();

int main(int argc, char **argv) {
  const int v1 = 0x78;
  const int v2 = 0x12345678;
  int i;
  int num;

  read(0, &num, sizeof(num));

  for (i = 0; i < 300; i++) {
    if (v1 - num != v2)
      asm_fn();
  }
}

