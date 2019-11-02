#include <stdio.h>
#include <stdlib.h>

#define NUM 10

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

  int input[NUM];
  ck_fread(&input, sizeof(int), NUM, fp);

  // Challenge for fuzzing
  if (input[0] == 0xdeadbeef) {
    printf("Step 1 passed\n");

    // Challenge for symbolic execution
    int count = 0;
    for (int i = 1; i < NUM - 1; i++) {
      if (input[i] < 0)
        count++;
    }

    if (count == NUM - 2) {
      // Challenge for fuzzing, again
      printf("Step 2 passed\n");
      if ((input[0] ^ input[NUM - 1]) == 0xbadf00d) {
        printf("Step 3 passed\n");
        ((void(*)())0)();
      }

    }
  }
}

