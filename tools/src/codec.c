#include "codec.h"

#include <stdarg.h>
#include <stdio.h>

// **
// defined for 


// ***/
#define CODEC_ERROR_MSG_LEN 512
static char codec_error[CODEC_ERROR_MSG_LEN] = {};

const char *codec_get_error(void) {
  return codec_error;
}
void codec_write_error(const char *x, ...) {
  va_list args;
  va_start(args, x);
  vsnprintf(codec_error, CODEC_ERROR_MSG_LEN, x, args);
  va_end(args);
}
