#include "util.h"

#include <stdio.h>
#include <stdlib.h>

extern int image_pack(void);

int main(int UNUSED_ARG(argv), char **UNUSED_ARG(args))
{
  printf("Start Core Tools\n");
  if (image_pack())
    return 1;
  printf("Completed Core Tools\n");
  return 0;
}