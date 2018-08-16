#include "prng.h"
#include "common.h"
#include <stdio.h>
#include <unistd.h>

int main() {
  uint64_t seed;
  char buf[0x10];
  char chal[sizeof(buf)];

  read(0, &seed, sizeof(seed));
  read(0, buf, sizeof(buf));
  sprng(seed);

  for (int i = 0; i < sizeof(chal); i++) {
    chal[i] = random_in_range('a', 'p');
    if (buf[i] != chal[i]) {
      bad();
      return 0;
    }
  }
  good();
}
