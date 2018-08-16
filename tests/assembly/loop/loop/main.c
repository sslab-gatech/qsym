#include <unistd.h>
#include "common.h"

int main() {
  int v = 0;
  int n;
  read(0, &n, sizeof(n));

  for (int i = 0; i < 0x1000; i++) {
    v += i * i;
  }

  if (n + v == 0xdeadbeef)
    good();
  else
    bad();
}
