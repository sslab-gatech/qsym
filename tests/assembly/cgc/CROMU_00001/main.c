#include "common.h"
#include <unistd.h>

int cgc_strcmp( const char *s1, const char *s2 )
{
    while ( *s1 && (*s1 == *s2) )
    {
      s1++,s2++;
    }
    return (*(const unsigned char *)s1 - *(const unsigned char *)s2);
}


int main() {
  char buf1[5], buf2[5];
  read(0, buf1, sizeof(buf1) - 1);
  read(0, buf2, sizeof(buf2) - 1);
  buf1[4] = buf2[4] = 0;

  if (!cgc_strcmp(buf1, buf2)) {
    if (buf1[0] == 'a')
      good();
  }
  else
    bad();
}
