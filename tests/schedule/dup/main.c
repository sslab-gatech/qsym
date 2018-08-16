#include <stdbool.h>
#include <unistd.h>

#include "common.h"

int main() {
  int x;

  read(0, &x, sizeof(x));

  if (x == 0xdeadbeef)
    good();
  else
    bad();
}
