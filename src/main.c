
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "noja.h"

int main()
{
	char *error_text;

	if(!run_file("samples/sample.noja", &error_text)) {

		fprintf(stderr, "Runtime error: %s\n", error_text);
		free(error_text);
		
	} else {

		fprintf(stderr, "All good.\n");
	}

	return 0;
}