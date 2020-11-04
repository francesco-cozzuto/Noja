
#include <stdlib.h>
#include <stdio.h>
#include "noja.h"

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

void token_array_print(token_array_t *array, char *source, FILE *fp)
{
	struct fp_and_source { FILE *fp; char *source; };

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

void token_array_deinit(token_array_t *array)
{
	token_chunk_t *chunk = array->head.next;

	while(chunk) {

		token_chunk_t *next = chunk->next;

		free(chunk);

		chunk = next;
	}
}