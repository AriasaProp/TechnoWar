#include <stdio.h>
#include <stdlib.h>

int main (void) {
  #if __has_builtin(__builtin_rotateleft32)
    printf("Has\n");
  #else
    printf("Hasn't\n");
  #endif
  return 0;
}