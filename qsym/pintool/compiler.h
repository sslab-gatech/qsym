#ifndef QSYM_COMPILER_H_
#define QSYM_COMPILER_H_

#define xglue(x, y) x ## y
#define glue(x, y) xglue(x, y)

#define likely(x)       __builtin_expect((x), 1)
#define unlikely(x)     __builtin_expect((x), 0)

inline void CRASH() {
  ((void(*)())0)();
}

#endif // QSYM_COMPILER_H_
