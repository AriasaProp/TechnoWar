#define _STB_LOCAL_IMPLEMENTATION_
#include "stb/local.h"

#include <stdarg.h>
#include <stdio.h>

#define STB_ERROR_MSG_LEN 1025
static char err_msg[STB_ERROR_MSG_LEN] = {0};

void stb_clean_error() {
  memset(err_msg, 0, STB_ERROR_MSG_LEN);
}
const char *stb_get_error() {
  return err_msg;
}
void stb_set_error(const char *x, ...) {
  va_list args;
  va_start(args, x);
  vsnprintf(err_msg, STB_ERROR_MSG_LEN, x, args);
  va_end(args);
}
