#include <unistd.h>
#include "common.h"

int main() {
  int n;
  read(0, &n, sizeof(n));

  for (int i = 0; i < 0x1000; i++) {
      n += i;
  }

  if (n == 0xdeadbeef)
    good();
  else
    bad();
}
