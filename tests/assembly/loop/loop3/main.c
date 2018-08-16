#include <unistd.h>
#include "common.h"

int main() {
  int v = 0;
  unsigned int n = 0;
  read(0, &n, sizeof(n));

  for (int i = 0; i < 0x1000; i++) {
      n = n + i;
  }

  if (n == 0xdf2db6ef)
    good();
  else
    bad();
}
