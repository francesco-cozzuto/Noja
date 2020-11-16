
#include <string.h>
#include <stdlib.h>
#include "noja.h"

#define OBJECT_TYPE(object) ((nj_object_type_t*) (object)->type)

int nj_collect_children(nj_state_t *state, nj_object_t *object)
{
	if(OBJECT_TYPE(object)->on_collect_children)
		return OBJECT_TYPE(object)->on_collect_children(state, object);

	return 1;
}

int nj_collect_object(nj_state_t *state, nj_object_t **reference)
{
	if(reference == NULL) 
		return 1;

	printf("collecting object at address %p\n", *reference);

	if((*reference)->flags & OBJECT_WAS_MOVED) {

		*reference = (*(nj_moved_object_t**) reference)->new_location;
		return 1;
	}
	
	// Align heap pointer

	if(state->heap.used & 7)
		state->heap.used = (state->heap.used & ~7) + 8;

	// Allocate

	size_t object_size = ((nj_object_type_t*) (*reference)->type)->size;

	printf("moving object with type name %s and size %ld\n", OBJECT_TYPE(*reference)->name, object_size);

	if(state->heap.used + object_size > state->heap.size) {

		// Out of heap
		return 0;
	}

	nj_object_t *object_copy = (nj_object_t*) (state->heap.chunk + state->heap.used);

	state->heap.used += object_size;

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

	printf("Collecting global variable maps\n");

	// Collect global variable maps
	{
		for(int i = 0; state->segments_used; i++)
			if(!nj_collect_object(state, &state->segments[i].global_variables_map))
				return 0;
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

int nj_collect(nj_state_t *state)
{
	nj_heap_t old_heap = state->heap;

	char *chunk = malloc(old_heap.size);

	if(chunk == 0)
		return 0;

	state->heap.chunk = chunk;
	state->heap.size = old_heap.size;
	state->heap.used = 0;

	if(!nj_collect_inner(state)) {

		free(state->heap.chunk);
		state->heap = old_heap;
		return 0;
	}

	// Call destructors of the objects into the old heap

	{
		size_t i = 0;

		while(i < old_heap.used) {

			if(i & 7)
				i = (i & ~7) + 8;

			nj_object_t *object = (nj_object_t*) (old_heap.chunk + i);

			nj_object_type_t *type = (nj_object_type_t*) object->type;

			if(type->super.flags & OBJECT_WAS_MOVED)
				type = (nj_object_type_t*) ((nj_moved_object_t*) type)->new_location;

			if(type->on_deinit)
				type->on_deinit(state, object);

			i += OBJECT_TYPE(object)->size;
		}

		overflow_allocation_t *p = old_heap.overflow_allocations;

		while(p) {

			nj_object_t *object = (nj_object_t*) p->body;

			nj_object_type_t *type = (nj_object_type_t*) object->type;

			if(type->super.flags & OBJECT_WAS_MOVED)
				type = (nj_object_type_t*) ((nj_moved_object_t*) type)->new_location;

			if(type->on_deinit)
				type->on_deinit(state, object);

			{
				overflow_allocation_t *prev_p = p->prev;
				free(p);
				p = prev_p;
			}
		}

		free(old_heap.chunk);
	}

	return 1;
}