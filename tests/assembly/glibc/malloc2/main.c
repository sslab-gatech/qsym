#include "common.h"
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
  int size1, size2;
  read(0, &size1, sizeof(size1));
  int* m1 = (int*)malloc(size1);
  read(0, &size2, sizeof(size2));
  int* m2 = (int*)malloc(size2);
  read(0, m2, size2);
  if (m2[0] == 0xdeadbeef)
    good();
  else
    bad();
}
