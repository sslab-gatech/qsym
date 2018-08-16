#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  int a, b;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  // avoid a trivial case
  if (b != 0 && -a == b)
    good();
  else
    bad();
}
