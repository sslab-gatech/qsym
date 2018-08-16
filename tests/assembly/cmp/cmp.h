#ifndef __CMP_H__
#define __CMP_H__

#include "common.h"

#define TEST_JCC(jcc)\
  do { \
    int n; \
    read(0, &n, sizeof(n)); \
    asm goto("cmp $0xdeadbeef, %0;" \
        #jcc " %l1;" \
        "jmp %l2;" \
        : \
        : "r"(n) \
        : \
        : good, bad); \
good: \
    good(); \
    goto done; \
bad: \
    bad(); \
done: \
    ; \
  } \
  while(0); \

#endif // __CMP_H__
