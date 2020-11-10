
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <noja.h>

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

nj_object_t *exposed_load_text(nj_state_t *state, size_t argc, nj_object_t **argv)
{
	if(argc != 1)
		return 0;

	const char *path;
	char *content;
	int   length;

	if(!nj_object_to_c_string(state, argv[0], &path, 0))
		return 0; // Was expecting a string!

	if(!load_text(path, &content, &length))
		return nj_get_null_object(state);

	return nj_object_from_c_string_ref_2(state, content, length);
}

nj_object_t *setup(nj_state_t *state)
{
	nj_object_t *variables = nj_object_istanciate(state, nj_get_dict_type_object(state));

	assert(variables);

	{
		nj_object_t *function_object = nj_object_from_c_function(state, exposed_load_text);

		assert(function_object);

		if(!nj_dictionary_insert(state, variables, "load_text", function_object))
			return 0;
	}

	return variables;
}