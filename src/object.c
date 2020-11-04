
#include <stdlib.h>
#include <string.h>
#include "noja.h"

enum {
	OBJECT_IS_COLLECTABLE = 1,
};

object_t *object_from_cint(state_t *state, int64_t value)
{
	object_t *o = object_istanciate(state, (object_t*) &int_type_object);

	object_int_t *x = (object_int_t*) o;

	x->value = value;

	return o;
}

object_t *object_from_cfloat(state_t *state, double value)
{
	object_t *o = object_istanciate(state, (object_t*) &float_type_object);

	object_float_t *x = (object_float_t*) o;

	x->value = value;

	return o;
}

uint8_t object_test(state_t *state, object_t *object)
{
	object_type_t *type = (object_type_t*) object;

	if(type->on_test)
		return type->on_test(state, object);

	return 0;
}

object_t *object_istanciate(state_t *state, object_t *type)
{

	size_t object_size = ((object_type_t*) type)->size;
	
	//
	// Align the heap pointer
	//

	if(state->heap_used & 7)
		state->heap_used = (state->heap_used & ~7) + 8;

	//
	// Determine if there is space in the heap or
	// this is an "overflowing allocation"
	//

	object_t *object;

	if(state->heap_used + object_size > state->heap_size) {

		overflow_allocation_t *allocation = malloc(sizeof(overflow_allocation_t) + object_size);

		if(allocation == 0)
			return 0;

		allocation->prev = state->overflow_allocations;
		state->overflow_allocations = allocation;
		
		object = (object_t*) allocation->body;
	
	} else {

		object = (object_t*) (state->heap + state->heap_used);

		state->heap_used += object_size;
	}
	
	//
	// Initialize the object
	//

	memset(object, 0, object_size);

	object->type = type;
	object->flags = OBJECT_IS_COLLECTABLE;

	if(((object_type_t*) type)->on_init)
		((object_type_t*) type)->on_init(state, object);

	return object;
}

void object_print(state_t *state, object_t *self, FILE *fp)
{
	object_type_t *t = (object_type_t*) self->type;

	if(t->on_print) {

		t->on_print(state, self, fp);
	
	} else {

		fprintf(fp, "<Unprintable object>");
	}
}

object_t *object_add(state_t *state, object_t *self, object_t *right)
{
	object_type_t *t = (object_type_t*) self->type;

	if(t->on_add == 0) {

		// #ERROR
		// Undefined ADD operation
		return 0;
	}

	return t->on_add(state, self, right);
}