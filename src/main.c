
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "noja.h"

int main()
{
	char buffer[4096];

	if(!run_file("samples/sample.noja", buffer, sizeof(buffer))) {

		fprintf(stderr, "Runtime error: %s\n", buffer);

	} else {

		fprintf(stderr, "All good.\n");
	}

	return 0;
}