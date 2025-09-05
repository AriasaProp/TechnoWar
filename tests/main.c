#include <stdlib.h>

#if !defined(PACK_TEST) && !defined(DECODER_TEST) && !defined(FAST_TEST)
#define PACK_TEST
#define DECODER_TEST
#define FAST_TEST
#endif

#ifdef PACK_TEST
extern int rect_pack_test(void);
#endif // PACK_TEST
#ifdef DECODER_TEST
extern int decoder_test(void);
#endif // DECODER_TEST
#ifdef FAST_TEST
extern int fast_test(void);
#endif // FAST_TEST

int main (int, char**) {
#ifdef PACK_TEST
  if (rect_pack_test())
    goto end_main;
#endif // PACK_TEST
#ifdef DECODER_TEST
  if (decoder_test())
    goto end_main;
#endif // DECODER_TEST
#ifdef FAST_TEST
  if (fast_test())
    goto end_main;
#endif // FAST_TES
  return EXIT_SUCCESS;
end_main:
  return EXIT_FAILURE;
}