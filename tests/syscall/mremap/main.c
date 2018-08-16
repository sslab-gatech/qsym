#define _GNU_SOURCE

#include "common.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

int main() {
  int* p = (int*)mmap (0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (p == MAP_FAILED)
    perror("mmap");
  read(0, p, sizeof(int));

  int *new_p = (int*)mremap(p, 0x1000, 0x2000, MREMAP_MAYMOVE);
  if (new_p[0] == 0xdeadbeef)
    good();
  else
    bad();
}
