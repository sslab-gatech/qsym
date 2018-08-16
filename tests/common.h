#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>

static inline void good() {
  printf("Good\n");
}

static inline void bad() {
  printf("Bad\n");
}

#ifdef __x86_64__
  #define MACHINE_AX "rax"
  #define MACHINE_CX "rcx"
  #define MACHINE_BX "rbx"
  #define MACHINE_DX "rdx"
#else
  // TODO: Add 32bit checking
  #define MACHINE_AX "eax"
  #define MACHINE_CX "ecx"
  #define MACHINE_BX "ebx"
  #define MACHINE_DX "edx"
#endif

#endif // __COMMON_H__
