#ifndef _STB_LOCAL_INCLUDED_
#define _STB_LOCAL_INCLUDED_

#define PNG_SIGNATURE "\x89PNG\r\n\x1A\n"
#define MAX_DIMENSIONS (1 << 24)

#ifndef _STB_LOCAL_IMPLEMENTATION_

extern void stb_clean_error();
extern const char *stb_get_error();
extern void stb_set_error(const char*,...);

#endif // _STB_LOCAL_IMPLEMENTATION_

#endif // _STB_LOCAL_INCLUDED_