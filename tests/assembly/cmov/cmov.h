#include "common.h"
#include <unistd.h>

#define CMOV_RM(ins)  \
  do { \
  int a, b, c;  \
  read(0, &a, sizeof(a)); \
\
  __asm__("cmpl $0xdeadbeef, %2\n"  \
      "movl $0xdeadbeef, %1\n"  \
      #ins " %1, %0\n"  \
      : "=r"(c), "=m"(b)  \
      : "m"(a));  \
\
  if (c == 0xdeadbeef)  \
    good(); \
  else  \
    bad();  \
} while(0);

#define CMOV_RR(ins)  \
  do { \
  int a, b, c;  \
  read(0, &a, sizeof(a)); \
\
  __asm__("cmpl $0xdeadbeef, %2\n"  \
      "movl $0xdeadbeef, %1\n"  \
      #ins " %1, %0\n"  \
      : "=r"(c), "=r"(b)  \
      : "r"(a));  \
\
  if (c == 0xdeadbeef)  \
    good(); \
  else  \
    bad();  \
} while(0);
