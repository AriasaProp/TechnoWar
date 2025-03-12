#include "util.h"

#include <stdlib.h>
#include <stdio.h>

extern int image_pack_test (void);

int main (int argv, char **args) {
	UNUSED(argv);
	UNUSED(args);
	printf("Start Core Test\n");
	if (image_pack_test ()) return 1;
	
	printf("Completed Core Test\n");
	return 0;
}