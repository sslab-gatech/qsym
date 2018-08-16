#include "common.h"
#include <unistd.h>
#include <string.h>

int cgc_atoi(char *s) {
  if (s == NULL) {
    return 0;
  }

  int negative = 0;
  int num_digits = 0;
  int number = 0;

  while(*s != '\0')
  {
    if ((*s == '-') && (num_digits==0)) {
      negative = 1;                                                                                                                                                                                                s++;
      continue;
    }
    if (*s < '0' || *s > '9') {
      return number;
    }
    number = (number * 10) + (*s - '0');
    num_digits++;
    s++;
    if (num_digits > 10) {
      return number;
    }
  }
  if (negative) {
    return (-1 * number);
  } else {
    return number;
  }
}

int main() {
  char buf[0x10];
  memset(buf, 0, sizeof(buf));
  read(0, buf, sizeof(buf) - 1);
  if (cgc_atoi(buf) == 1234)
    good();
  else
    bad();
}
