
#include <stdlib.h>
#include "noja.h"

executable_t *compile(char *source, int length, char *error_buffer, int error_buffer_size)
{

	token_array_t tokens;
	pool_t *pool;
	node_t *node;

	if(!tokenize(source, length, &tokens))

		// Failed to tokenize
		return 0;

	printf("tokenized\n");

	if(!parse(&tokens, source, &pool, &node))

		// Failed to parse
		return 0;

	printf("parsed\n");

	if(!check(node, source, error_buffer, error_buffer_size))

		// There was a semantic error!
		return 0;

	printf("checked\n");

	executable_t *executable = generate(node);

	pool_destroy(pool);
	token_array_deinit(&tokens);

	return executable;
}

executable_t *compile_from_file(char *path, char *error_buffer, int error_buffer_size)
{
	char *source;
	int length;

	if(!load_text(path, &source, &length))

		// Failed to load file contents
		return 0;

	executable_t *executable = compile(source, length, error_buffer, error_buffer_size);

	free(source);

	return executable;
}