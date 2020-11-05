
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "noja.h"

int main(int argc, char **argv)
{
	if(argc == 1) {

		fprintf(stderr, "A file path was expected\n");
		return -1;
	}

	char *error_text;

	if(!run_file(argv[1], &error_text)) {

		fprintf(stderr, "%s\n", error_text);
		free(error_text);
	}

	return 0;
}