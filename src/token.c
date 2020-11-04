
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"

int64_t token_to_int(token_t token, char *text)
{
	assert(token.kind == TOKEN_KIND_VALUE_INT);

	int64_t value;


	char c = text[token.offset + token.length];
	text[token.offset + token.length] = '\0';

	value = strtoll(text + token.offset, 0, 10);

	text[token.offset + token.length] = c;
	return value;
}

double token_to_float(token_t token, char *text)
{
	assert(token.kind == TOKEN_KIND_VALUE_FLOAT);

	double value;


	char c = text[token.offset + token.length];
	text[token.offset + token.length] = '\0';

	value = strtod(text + token.offset, 0);

	text[token.offset + token.length] = c;
	return value;
}

int token_to_string(pool_t *pool, token_t token, char *text, char **e_result, int *e_length)
{
	assert(token.kind == TOKEN_KIND_VALUE_STRING || token.kind == TOKEN_KIND_IDENTIFIER);

	if(token.kind == TOKEN_KIND_VALUE_STRING) {

		char *result = pool_request(pool, token.length - 2 + 1);
		int   length = token.length - 2;

		if(result == 0)
			return 0;

		memcpy(result, text + token.offset + 1, length);
		result[length] = '\0';

		*e_result = result;

		if(e_length)
			*e_length = length;

	} else {

		char *result = pool_request(pool, token.length + 1);
		int   length = token.length;

		if(result == 0)
			return 0;

		memcpy(result, text + token.offset, length);
		result[length] = '\0';

		*e_result = result;

		if(e_length)
			*e_length = length;

	}

	return 1;
}