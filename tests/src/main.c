#include "util.h"

#include <stdio.h>
#include <stdlib.h>

// stb test group
extern int rect_pack_test();
extern int image_codec_test();


int main(int UNUSED_ARG(argv), char **UNUSED_ARG(args)) {
  printf("Core Test\n");
  
  printf(" STB Image:\n");
  int result = 
    rect_pack_test() ||
    image_codec_test();
  
  return result;
}