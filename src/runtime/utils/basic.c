
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

int load_text(const char *path, char **e_content, int *e_length)
{
	FILE *fp = fopen(path, "rb");

	if(fp == 0)
		return 0;

	size_t length;

	fseek(fp, 0, SEEK_END);

	length = ftell(fp);
	
	if(e_length)
		*e_length = length;

	fseek(fp, 0, SEEK_SET);

	*e_content = malloc(length + 1);

	if(*e_content == 0) {

		fclose(fp);
		return 0;
	}

	if(fread(*e_content, 1, length, fp) != length) {

		free(*e_content);
		
		*e_content = 0;
		
		if(e_length)
			*e_length = 0;

		fclose(fp);
		return 0;
	}

	(*e_content)[length] = '\0';

	fclose(fp);
	return 1;
}