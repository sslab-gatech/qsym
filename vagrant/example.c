#include <stdio.h>
#include <stdlib.h>

void ck_fread(void* ptr, size_t size, size_t nitems, FILE *stream) {
  if (fread(ptr, size, nitems, stream) != nitems) {
    printf("[-] Failed to read\n");
    exit(-1);
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s [input]\n", argv[0]);
    exit(-1);
  }

  FILE* fp = fopen(argv[1], "rb");

  if (fp == NULL) {
    printf("[-] Failed to open\n");
    exit(-1);
  }

  int x, y;
  char buf[32];

  ck_fread(&x, sizeof(x), 1, fp);
  ck_fread(buf, 1, sizeof(buf), fp);
  ck_fread(&y, sizeof(y), 1, fp);

  // Challenge for fuzzing
  if (x == 0xdeadbeef) {
    printf("Step 1 passed\n");

    // Challenge for symbolic execution
    int count = 0;
    for (int i = 0; i < 32; i++) {
      if (buf[i] >= 'a')
        count++;
    }

    if (count >= 8) {
      printf("Step 2 passed\n");

      // Challenge for fuzzing, again
      if ((x ^ y) == 0xbadf00d) {
        printf("Step 3 passed\n");
        ((void(*)())0)();
      }
    }
  }
}
