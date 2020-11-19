
#include <string.h>
#include <stdlib.h>
#include "noja.h"

#define OBJECT_TYPE(object) ((nj_object_type_t*) (object)->type)

int nj_collect_children(nj_state_t *state, nj_object_t *object)
{
	if(object->type != object)
		if(!nj_collect_object(state, &object->type))
			return 0;

	if(OBJECT_TYPE(object)->on_collect_children)
		return OBJECT_TYPE(object)->on_collect_children(state, object);

	return 1;
}

int nj_collect_object(nj_state_t *state, nj_object_t **reference)
{

	if(reference == NULL) {

		return 1;
	}

	if(*reference == NULL) {

		return 1;
	}

	if((*reference)->flags & OBJECT_WAS_MOVED) {

		*reference = (*(nj_moved_object_t**) reference)->new_location;

		return 1;
	}
	
	if(!((*reference)->flags & OBJECT_IS_COLLECTABLE))

		return nj_collect_children(state, *reference);

	// Align heap pointer

	if(state->temp_heap.used & 7)
		state->temp_heap.used = (state->temp_heap.used & ~7) + 8;

	// Allocate

	nj_update_reference(&(*reference)->type);

	size_t object_size = ((nj_object_type_t*) (*reference)->type)->size;

	if(state->temp_heap.used + object_size > state->temp_heap.size)

		// Out of heap
		return 0;

	nj_object_t *object_copy = (nj_object_t*) (state->temp_heap.chunk + state->temp_heap.used);

	state->temp_heap.used += object_size;

	// Copy the object in the newly allocated space

	memcpy(object_copy, *reference, object_size);

	// Set the old object copy and set the new location pointer

	(*reference)->flags |= OBJECT_WAS_MOVED;

	(*(nj_moved_object_t**) reference)->new_location = object_copy;

	// Update the parent pointer

	*reference = object_copy;

	return nj_collect_children(state, object_copy);
}

int nj_should_collect(nj_state_t *state)
{
	return !!state->heap.overflow_allocations;
}

static int nj_collect_inner(nj_state_t *state)
{

	printf("Collecting global variable maps (there are %d)\n", state->segments_used);

	// Collect global variable maps
	{
		for(int i = 0; i < state->segments_used; i++) {

			if(!nj_collect_object(state, &state->segments[i].global_variables_map))
				return 0;
		}
	}

	printf("Collecting variable maps\n");

	// Collect variable maps
	{
		object_stack_chunk_t *chunk = state->vars_stack.tail;

		for(size_t i = 0; i < state->vars_stack.relative_size; i++)
			if(!nj_collect_object(state, &chunk->items[i]))
				return 0;

		chunk = chunk->prev;

		while(chunk) {

			for(size_t i = 0; i < OBJECT_STACK_ITEMS_PER_CHUNK; i++)
				if(!nj_collect_object(state, &chunk->items[i]))
					return 0;

			chunk = chunk->prev;
		}
	}

	printf("Collecting stack\n");

	// Collect stack
	{
		object_stack_chunk_t *chunk = state->eval_stack.tail;

		for(size_t i = 0; i < state->eval_stack.relative_size; i++)
			if(!nj_collect_object(state, &chunk->items[i]))
				return 0;

		chunk = chunk->prev;

		while(chunk) {

			for(size_t i = 0; i < OBJECT_STACK_ITEMS_PER_CHUNK; i++)
				if(!nj_collect_object(state, &chunk->items[i]))
					return 0;

			chunk = chunk->prev;
		}
	}

	return 1;
}

void nj_update_reference(nj_object_t **reference)
{

	if((*reference)->flags & OBJECT_WAS_MOVED)
		(*reference) = ((nj_moved_object_t*) *reference)->new_location;
}

int nj_collect(nj_state_t *state)
{
	char *chunk = malloc(state->heap.size);

	if(chunk == 0)
		return 0;

	state->temp_heap.chunk = chunk;
	state->temp_heap.size = state->heap.size;
	state->temp_heap.used = 0;
	state->temp_heap.overflow_allocations = NULL;

	if(!nj_collect_inner(state)) {

		free(state->temp_heap.chunk);
		return 0;
	}

	// Call destructors of the objects into the old heap

	{
		size_t i = 0;

		while(i < state->heap.used) {

			if(i & 7)
				i = (i & ~7) + 8;

			nj_object_t *object = (nj_object_t*) (state->heap.chunk + i);

			if(!(object->flags & OBJECT_WAS_MOVED)) {

				nj_update_reference(&object->type);

				nj_object_type_t *type = (nj_object_type_t*) object->type;

				if(type->on_deinit)
					type->on_deinit(state, object);

			}

			i += OBJECT_TYPE(object)->size;
		}

		overflow_allocation_t *p = state->heap.overflow_allocations;

		while(p) {

			nj_object_t *object = (nj_object_t*) p->body;

			nj_update_reference(&object->type);

			nj_object_type_t *type = (nj_object_type_t*) object->type;

			if(type->on_deinit)
				type->on_deinit(state, object);

			{
				overflow_allocation_t *prev_p = p->prev;
				free(p);
				p = prev_p;
			}
		}

		free(state->heap.chunk);
	}

	state->heap = state->temp_heap;

	return 1;
}