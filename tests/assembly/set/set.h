#include "common.h"
#include <unistd.h>

#define GET_FLAG(set) { \
  int n, v = 0; \
  read(0, &n, sizeof(n)); \
  do { \
    __asm("cmp $0xdeadbeef, %1;" \
        #set" %0;" \
        : "=m"(v) \
        : "r"(n)); \
  } \
  while(0); \
  if (v) \
    good(); \
  else \
    bad(); \
}

