#include "common.h"
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
  int size;
  read(0, &size, sizeof(size));
  int* m = (int*)malloc(size);
  read(0, m, size);
  if (m[0] == 0xdeadbeef)
    good();
  else
    bad();
}
