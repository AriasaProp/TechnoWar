#include "util.h"

#include <stdio.h>
#include <stdlib.h>

extern int stb_test(void);

int main(int UNUSED_ARG(argv), char **UNUSED_ARG(args)) {
  printf("Start Core Test\n");
  int result = 0;
  result = stb_test();
  return result;
}