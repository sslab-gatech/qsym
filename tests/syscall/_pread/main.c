#define _XOPEN_SOURCE 500

#include "common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv) {
  int fd = open(argv[1], O_RDONLY);
  int a;
  pread(fd, &a, 4, sizeof(a));

  if (a == 0xdeadbeef)
    good();
  else
    bad();
}
