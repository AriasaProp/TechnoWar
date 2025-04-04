#include "utility/image_manager.h"
#include "util.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define PNG_SIGNATURE "\x89PNG\r\n\x1A\n"

typedef union {
	uint8_t ub[1024];
	uint16_t us[512];
	uint32_t ui[256];
	uint64_t ul[128];
} code;

static char err_msg[1025] = {};
static inline void err_call(const char *x, ...) {
	static char msg[512];
  va_list args;
  va_start(args, x);
  vsnprintf(msg, 512, x, args);
  va_end(args);
  
	size_t msg_len = strlen(msg);
	msg[msg_len] = '\n';
	++msg_len;
	memmove(err_msg + msg_len, err_msg, 1024 - msg_len);
	memcpy(err_msg, msg, msg_len);
}
static inline void err_clean() {
	memset(err_msg, 0, 1024);
}

typedef size_t (*read_callback) (void*,uint8_t *,size_t);
typedef void (*seek_callback) (void*,long);
typedef int (*eof_callback) (void*);

//helper

// decode have rule
// return 1 when signature match
// return 2 when data valid

static int png_decode(im_image *out, void *c, read_callback rc, seek_callback sc, eof_callback re) {
	UNUSED(sc);
	code buffer;
	size_t i;
	// decode
	uint32_t chunk_len;
	int color_type, compressed_method, filter_method, interlace;
	int result = 0;
	int internal = 0; // 1 header √, 2 idata √
#define READABLE32(X) (((X<<24)&0xff) | ((X<<16)&0xff) | ((X>>8)&0xff) | ((X>>24)&0xff))
#define CHUNK_PARSE(A,B,C,D) (A|(B<<8)|(C<<16)|(D<<25))
//force read till eof
#define FORCE_READ(C, B, R, E, L) \
	i = L; \
	while (i -= R(C, B + i, i)) if (E(C)) \
		goto png_parse_end

	do {
		FORCE_READ(c, buffer.ub, rc,re, 8);
		switch (buffer.ui[1]) {
			case CHUNK_PARSE('\r','\n','\x1A','\n'): // check signature
				if (memcmp(buffer.ub, PNG_SIGNATURE, 8)) return 0;
				result |= 1;
				break;
			case CHUNK_PARSE('I','H','D','R'): { // check header
				chunk_len = READABLE32(buffer.ui[0]);
				if (chunk_len != 13) {
					err_call("PNG Header chunk length %d", chunk_len);
					goto png_parse_end;
				}
				FORCE_READ(c, buffer.ub, rc, re, chunk_len);
				out->width = READABLE32(buffer.ui[0]);
				out->height = READABLE32(buffer.ui[1]);
				out->bits_per_channel = buffer.ub[8];
				color_type = buffer.ub[9];
				compressed_method = buffer.ub[10];
				filter_method = buffer.ub[11];
				interlace = buffer.ub[12];
				if (!(out->height * out->width)) {
					err_call("PNG Header: width %d, height %d error",  out->width,out->height);
					goto png_parse_end;
				}
				switch (color_type) {
			  	case 0:
						if (!out->bits_per_channel || (out->bits_per_channel > 16) || (out->bits_per_channel & (out->bits_per_channel - 1))) {
							err_call("PNG Header: depth %d color type %d",  out->bits_per_channel, color_type);
							goto png_parse_end;
						}
			  		break;
			  	case 2:
			  	case 4:
			  	case 6:
			  		if ((out->bits_per_channel != 8) && (out->bits_per_channel != 16)) {
							err_call("PNG Header: depth %d color type %d",  out->bits_per_channel, color_type);
							goto png_parse_end;
						}
			  		break;
			  	case 3: // indexing image
						if (!out->bits_per_channel || (out->bits_per_channel >= 16) || (out->bits_per_channel & (out->bits_per_channel - 1))) {
							err_call("PNG Header: depth %d color type %d",  out->bits_per_channel, color_type);
							goto png_parse_end;
						}
			  		//pal_img_n = 3;
			  		break;
			  	default:
						err_call("PNG Header: color type %d unknown", color_type);
						goto png_parse_end;
			  }
			  if (compressed_method || filter_method || (interlace > 1)) {
					err_call("PNG Header: unsupported method");
					goto png_parse_end;
			  }
				internal |= 1; // header √
			}
				break;
		}
	} while (!re(c));
	result |= 2;
#undef FORCE_READ
#undef READABLE32
#undef CHUNK_PARSE
	UNUSED(internal);
png_parse_end:
	return result;
}

//image reader
static im_image im_image_read(void *c, read_callback rc, seek_callback sc, eof_callback re) {
	im_image out;
	
	// try all decode when only decoder before is return 0
	// when no matching decoder, at least match signature return error
	if (
		png_decode(&out, c, rc, sc, re) ||
		0
	) {
		return out;
	}
	
	
	err_call("no matching any decode!");
	return out;
}

//image writer




static size_t im_memory_read(void *c, uint8_t *buff, size_t len) {
	const unsigned char **mc = (const unsigned char**)c;
	len = MIN((long)len, mc[2] - mc[1]);
	memcpy(buff, mc[1], len);
	mc[1] += len;
	return len;
}
static void im_memory_seek(void *c, long len) {
	const unsigned char **mc = (const unsigned char**)c;
	mc[1] = CLAMP(mc[0], mc[2], mc[1] + len);
}
static int im_memory_eof(void *c) {
	const unsigned char **mc = (const unsigned char**)c;
	return (mc[1] < mc[2]);
}

static size_t im_file_read(void *c, uint8_t *buff, size_t len) {
	return fread(buff, 1, len, (FILE*)c);
}
static void im_file_seek(void *c, long len) {
	fseek((FILE*)c, len, SEEK_CUR);
}
static int im_file_eof(void *c) {
	return feof((FILE*)c);
}




im_image im_read_image_from_mem(const unsigned char *bytes, unsigned int len) {
	const unsigned char *c[3] = {bytes, bytes, bytes + len};
	return im_image_read(c, im_memory_read, im_memory_seek, im_memory_eof);
}
im_image im_read_image_from_file(const char *filename) {
	FILE *f = fopen(filename, "wb");
	im_image out = im_image_read((void*)f, im_file_read, im_file_seek, im_file_eof);
	fclose (f);
	return out;
}

void *im_write_image_to_mem(im_image o, im_image_format f, unsigned int *outlen) {
	UNUSED(o);
	UNUSED(f);
	UNUSED(outlen);
	return 0;
}
int im_write_image_to_file(const char *filename, im_image o, im_image_format f) {
	UNUSED(filename);
	UNUSED(o);
	UNUSED(f);
	
	
	return 1;
}

void im_free_image(im_image o) {
	free(o.bitmap);
	o.bitmap = 0;
}