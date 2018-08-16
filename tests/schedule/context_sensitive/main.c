#include <stdbool.h>
#include <unistd.h>

#include "common.h"

int main() {
  int x, y, z;
  bool flag = false;

  read(0, &x, sizeof(x));
  read(0, &y, sizeof(y));
  read(0, &z, sizeof(z));

  if (x == 0xdeadbeef)
    flag = true;

  if (y == 0xcafebabe)
    flag = false;

  if (z == 0xbadf00d)
  {
    if (flag)
      good();
  }
}
