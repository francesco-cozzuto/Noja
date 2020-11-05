
#include "noja.h"

void token_iterator_init(token_iterator_t *iterator, token_array_t *array)
{
	iterator->chunk = &array->head;
	iterator->absolute_offset = 0;
	iterator->relative_offset = 0;
	iterator->count = array->count;
}

int __token_iterator_next(token_iterator_t *iterator, char *file, int line, const char *func, char *source)
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

	{
		(void) file;
		(void) line;
		(void) func;
		
		token_t token = iterator->chunk->tokens[iterator->relative_offset];
		printf(">> Now at [");
		for(int i = 0; i < token.length; i++)
			printf("%c", source[token.offset + i]);
		printf("] [%d, %d] from %s:%d in %s\n", token.offset, token.length, file, line, func);
	}
	
	return 1;
}

int __token_iterator_prev(token_iterator_t *iterator, char *file, int line, const char *func, char *source)
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

	(void) file;
	(void) line;
	(void) func;

	token_t token = iterator->chunk->tokens[iterator->relative_offset];
	printf(">> Back at [");
		for(int i = 0; i < token.length; i++)
			printf("%c", source[token.offset + i]);
		printf("] [%d, %d] from %s:%d in %s\n", token.offset, token.length, file, line, func);
	return 1;
}

token_t token_iterator_current(token_iterator_t *iterator)
{
	return iterator->chunk->tokens[iterator->relative_offset];
}