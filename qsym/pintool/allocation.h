#ifndef QSYM_ALLOCATION_H_
#define QSYM_ALLOCATION_H_

#include <sys/mman.h>
#include "logging.h"

namespace qsym {

void* allocPages(size_t, int);
void* allocRWPages(size_t);
void  deallocPages(void*, size_t);
void* safeRealloc(void*, size_t);
void* safeMalloc(size_t);
void* safeCalloc(size_t, size_t);

} // namespace qsym

#endif // QSYM_ALLOCATION_H_
