#define COMMON_IMPLEMENTATION_
#include "common.h"

// helper
#ifdef _WIN32
int convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t *input) {
  return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, NULL, NULL);
}
#endif

void flipBytes(uint8_t *b, const size_t l) {
  if (l <= 1) return;
  for (size_t i = 0, j = l - 1, k = l >> 1; i < k; ++i, --j) {
    b[i] ^= b[j];
    b[j] ^= b[i];
    b[i] ^= b[j];
  }
}