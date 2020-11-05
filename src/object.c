
#include <stdlib.h>
#include <string.h>
#include "noja.h"

enum {
	OBJECT_IS_COLLECTABLE = 1,
};

object_t *object_from_cint(state_t *state, int64_t value)
{
	object_t *o = object_istanciate(state, (object_t*) &int_type_object);

	if(o == 0)
		return 0;

	object_int_t *x = (object_int_t*) o;

	x->value = value;

	return o;
}

object_t *object_from_cfloat(state_t *state, double value)
{
	object_t *o = object_istanciate(state, (object_t*) &float_type_object);

	if(o == 0)
		return 0;

	object_float_t *x = (object_float_t*) o;

	x->value = value;

	return o;
}

object_t *object_from_executable_and_offset(state_t *state, executable_t *executable, uint32_t offset)
{
	object_t *o = object_istanciate(state, (object_t*) &function_type_object);

	if(o == 0)
		return 0;

	object_function_t *x = (object_function_t*) o;

	x->executable = executable;
	x->offset = offset;

	return o;
}

object_t *object_select(state_t *state, object_t *self, object_t *key)
{

	object_type_t *type = (object_type_t*) self->type;
	
	if(type->on_select == 0)
		return 0;

	return type->on_select(state, self, key);
}

int object_insert(state_t *state, object_t *self, object_t *key, object_t *item)
{
	object_type_t *type = (object_type_t*) self->type;

	if(type->on_insert == 0)
		return 0;

	return type->on_insert(state, self, key, item);
}

object_t *object_select_attribute(state_t *state, object_t *self, const char *name)
{
	object_type_t *type = (object_type_t*) self->type;

	if(type->on_select_attribute) {

		object_t *selected = type->on_select_attribute(state, self, name);
	
		if(selected != 0)
			return selected;
	}

	if(type->methods == 0)
		return 0;

	return dict_cselect(state, type->methods, name);
}

int object_insert_attribute(state_t *state, object_t *self, const char *name, object_t *value)
{
	object_type_t *type = (object_type_t*) self->type;

	if(type->on_insert_attribute) {

		if(type->on_insert_attribute(state, self, name, value))
			return 1;
	}

	if(type->methods == 0) {

		object_t *dict = object_istanciate(state, (object_t*) &dict_type_object);

		if(dict == 0)
			return 0;

		type->methods = dict;
	}

	return dict_cinsert(state, type->methods, name, value);
}



uint8_t object_test(state_t *state, object_t *object)
{
	object_type_t *type = (object_type_t*) object->type;

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