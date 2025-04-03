#include "util.h"

#include <stdlib.h>
#include <stdio.h>

extern int stb_test (void);

int main (int UNUSED_ARG(argv), char **UNUSED_ARG(args)) {
	printf("Start Core Test\n");
	int result = 0;
	result = stb_test ();
	return result;
}