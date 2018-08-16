#include "common.h"
#include <unistd.h>
#include <string.h>

int main(void) {
  char src[0x100];
  char dst[0x100];

  read(0, src, sizeof(src));
  memset(dst, 0, sizeof(dst));

  __asm__(
      "rep movsd"
      :
      : "S"(src), "D"(dst), "c"(0x100 / 4));

  if (dst[0x80] == '?')
    good();
  else
    bad();
}
