#include <unistd.h>
#include "common.h"
#include "prng.h"

int main() {
  uint64_t seed;
  read(0, &seed, sizeof(seed));
  sprng(seed);
  uint32_t rng = prng();
  uint32_t guess;
  read(0, &guess, sizeof(guess));
  if (guess == rng)
    good();
  else
    bad();
}
