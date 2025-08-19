#define DECODER_IMPLEMENTATIONS_
#include "decoder.h"

#include <stdarg.h>
#include <stdio.h>

// **
// defined for

// ***/
#define DECODER_ERROR_MSG_LEN 512
static char decoder_error[DECODER_ERROR_MSG_LEN] = {};

const char *decoder_get_error(void) {
  return decoder_error;
}
void decoder_write_error(const char *x, ...) {
  va_list args;
  va_start(args, x);
  vsnprintf(decoder_error, DECODER_ERROR_MSG_LEN, x, args);
  va_end(args);
}
