#include <stdio.h>

extern int rect_pack_test();
extern int image_codec_test();


int stb_test() {
	printf("STB Test\n");
	int res = 0;
	
	res = rect_pack_test();
	res = image_codec_test();
	
	return res;
}