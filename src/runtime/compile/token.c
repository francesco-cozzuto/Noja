
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"

int64_t token_to_int(token_t token, const char *text)
{
	assert(token.kind == TOKEN_KIND_VALUE_INT);

	int64_t value;

	char *buffer = alloca(token.length + 1);

	memcpy(buffer, text + token.offset, token.length);
	buffer[token.length] = '\0';

	value = strtoll(buffer, 0, 10);

	return value;
}

double token_to_float(token_t token, const char *text)
{
	assert(token.kind == TOKEN_KIND_VALUE_FLOAT);

	double value;

	char *buffer = alloca(token.length + 1);

	memcpy(buffer, text + token.offset, token.length);
	buffer[token.length] = '\0';

	value = strtod(text + token.offset, 0);
	
	return value;
}

int token_to_string(pool_t *pool, token_t token, const char *text, char **e_result, int *e_length)
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

void token_array_init(token_array_t *array)
{
	array->head.prev = 0;
	array->head.next = 0;
	array->head.used = 0;
	array->tail = &array->head;
	array->count = 0;
}

int token_array_push(token_array_t *array, token_t token)
{
	if(array->tail->used == TOKENS_PER_CHUNK) {

		token_chunk_t *chunk = malloc(sizeof(token_chunk_t));

		if(chunk == 0)
			return 0;

		chunk->prev = array->tail;
		chunk->next = 0;
		chunk->used = 0;

		array->tail->next = chunk;
		array->tail = chunk;
	}

	array->tail->tokens[array->tail->used++] = token;
	array->count++;

	return 1;
}

void token_array_foreach(token_array_t *array, void *userdata, int (*callback)(void *data, int index, token_t token))
{
	token_chunk_t *chunk = &array->head;

	int i = 0;

	while(chunk) {

		for(int j = 0; j < chunk->used; j++, i++)
			if(!callback(userdata, i, chunk->tokens[j]))
				return;

		chunk = chunk->next;
	}
}

/*
void token_array_print(token_array_t *array, const char *source, FILE *fp)
{
	struct fp_and_source { FILE *fp; const char *source; };

	int callback(void *data, int index, token_t token) {

		(void) index;

		FILE *fp = ((struct fp_and_source*) data)->fp;
		char *source = ((struct fp_and_source*) data)->source;

		char t = source[token.offset + token.length];
		source[token.offset + token.length] = '\0';

		fprintf(fp, ">> token [%s], offset %d, length %d, kind %d, \n", source + token.offset, token.offset, token.length, token.kind);

		source[token.offset + token.length] = t;
		return 1;
	}

	struct fp_and_source data;
	data.fp = fp;
	data.source = source;

	token_array_foreach(array, (void*) &data, callback);
}
*/

void token_array_deinit(token_array_t *array)
{
	token_chunk_t *chunk = array->head.next;

	while(chunk) {

		token_chunk_t *next = chunk->next;

		free(chunk);

		chunk = next;
	}
}


void token_iterator_init(token_iterator_t *iterator, token_array_t *array)
{
	iterator->chunk = &array->head;
	iterator->absolute_offset = 0;
	iterator->relative_offset = 0;
	iterator->count = array->count;
}

int token_iterator_next(token_iterator_t *iterator)
{
	iterator->relative_offset++;
	iterator->absolute_offset++;

	if(iterator->absolute_offset == iterator->count) {

		iterator->relative_offset--;
		iterator->absolute_offset--;
		return 0;
	}

	if(iterator->relative_offset == iterator->chunk->used) {

		iterator->chunk = iterator->chunk->next;
		iterator->relative_offset = 0;
	}
	
	return 1;
}

int token_iterator_prev(token_iterator_t *iterator)
{
	iterator->relative_offset--;
	iterator->absolute_offset--;

	if(iterator->absolute_offset == -1) {

		iterator->relative_offset++;
		iterator->absolute_offset++;
		return 0;
	}

	if(iterator->relative_offset == -1) {

		iterator->chunk = iterator->chunk->prev;
		iterator->relative_offset = 0;
	}

	return 1;
}

token_t token_iterator_current(token_iterator_t *iterator)
{
	return iterator->chunk->tokens[iterator->relative_offset];
}