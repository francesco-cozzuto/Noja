
#include <stdlib.h>
#include "noja.h"

void object_stack_init(object_stack_t *stack)
{
	stack->head.prev = 0;
	stack->tail = &stack->head;
	stack->relative_size = 0;
	stack->absolute_size = 0;
}

void object_stack_deinit(object_stack_t *stack)
{
	object_stack_chunk_t *chunk = stack->tail;

	while(chunk->prev) {

		object_stack_chunk_t *prev = chunk->prev;

		free(chunk);

		chunk = prev;
	}
}

object_t *object_nth_from_top(object_stack_t *stack, int count)
{
	object_stack_chunk_t *chunk = stack->tail;
	int size = stack->relative_size;

	while(chunk) {

		if(count > size) {

			count -= size;

			chunk = chunk->prev;
			size = OBJECT_STACK_ITEMS_PER_CHUNK;

		} else {

			return chunk->items[size - count];
		}
	}

	return 0;
}

void object_stack_print(state_t *state, object_stack_t *stack, FILE *fp)
{
	object_stack_chunk_t *chunk = stack->tail;

	int j = stack->absolute_size-1;

	for(int i = 0; i < stack->relative_size; i++, j--) {
		fprintf(fp, "%d: ", j);
		object_print(state, chunk->items[stack->relative_size - i - 1], fp);
		fprintf(fp, "\n");
	}

	chunk = chunk->prev;

	while(chunk) {

		for(int i = 0; i < OBJECT_STACK_ITEMS_PER_CHUNK; i++, j--) {
			fprintf(fp, "%d: ", j);
			object_print(state, chunk->items[stack->relative_size - i - 1], fp);
			fprintf(fp, "\n");
		}

		chunk = chunk->prev;
	}
}

int object_stack_size(object_stack_t *stack)
{
	return stack->absolute_size;
}

int object_push(object_stack_t *stack, object_t *item)
{
	if(stack->relative_size == OBJECT_STACK_ITEMS_PER_CHUNK) {

		object_stack_chunk_t *chunk = malloc(sizeof(object_stack_chunk_t));

		if(chunk == 0)
			return 0;

		chunk->prev = stack->tail;
		stack->tail = chunk;
		stack->relative_size = 0;
	}

	stack->tail->items[stack->relative_size] = item;
	stack->relative_size++;
	stack->absolute_size++;

	return 1;
}

object_t *object_pop(object_stack_t *stack)
{
	if(stack->absolute_size == 0)
		return 0;

	object_t *popped = stack->tail->items[--stack->relative_size];
	stack->absolute_size--;

	if(stack->relative_size == 0) {

		object_stack_chunk_t *prev = stack->tail->prev;

		if(prev) {

			free(stack->tail);

			stack->tail = prev;
			stack->relative_size = OBJECT_STACK_ITEMS_PER_CHUNK;

		}
	}

	return popped;
}

object_t *object_top(object_stack_t *stack)
{
	return stack->tail->items[stack->relative_size-1];
}

object_t **object_top_ref(object_stack_t *stack)
{
	return stack->tail->items + stack->relative_size - 1;
}

void u32_stack_init(u32_stack_t *stack)
{
	stack->head.prev = 0;
	stack->tail = &stack->head;
	stack->relative_size = 0;
	stack->absolute_size = 0;
}

void u32_stack_deinit(u32_stack_t *stack)
{
	u32_stack_chunk_t *chunk = stack->tail;

	while(chunk->prev) {

		u32_stack_chunk_t *prev = chunk->prev;

		free(chunk);

		chunk = prev;
	}
}

int u32_stack_size(u32_stack_t *stack)
{
	return stack->absolute_size;
}

int u32_push(u32_stack_t *stack, uint32_t item)
{
	if(stack->relative_size == U32_STACK_ITEMS_PER_CHUNK) {

		u32_stack_chunk_t *chunk = malloc(sizeof(u32_stack_chunk_t));

		if(chunk == 0)
			return 0;

		chunk->prev = stack->tail;
		stack->tail = chunk;
		stack->relative_size = 0;
	}

	stack->tail->items[stack->relative_size] = item;
	stack->relative_size++;
	stack->absolute_size++;

	return 1;
}

uint32_t u32_pop(u32_stack_t *stack)
{
	if(stack->absolute_size == 0)
		return 0;

	uint32_t popped = stack->tail->items[--stack->relative_size];
	stack->absolute_size--;

	if(stack->relative_size == 0) {

		u32_stack_chunk_t *prev = stack->tail->prev;

		if(prev) {

			free(stack->tail);

			stack->tail = prev;
			stack->relative_size = U32_STACK_ITEMS_PER_CHUNK;

		}
	}

	return popped;
}

uint32_t u32_top(u32_stack_t *stack)
{
	return stack->tail->items[stack->relative_size-1];
}

uint32_t *u32_top_ref(u32_stack_t *stack)
{
	return stack->tail->items + stack->relative_size - 1;
}