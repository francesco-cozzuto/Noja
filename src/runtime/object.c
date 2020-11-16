
#include <stdlib.h>
#include <string.h>
#include "noja.h"

enum {
	OBJECT_IS_COLLECTABLE = 1,
};

nj_object_t *nj_get_dict_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_dict;
}

nj_object_t *nj_get_int_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_int;
}

nj_object_t *nj_get_float_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_float;
}

nj_object_t *nj_get_array_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_array;
}

nj_object_t *nj_get_bool_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_bool;
}

nj_object_t *nj_get_type_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_type;
}

nj_object_t *nj_get_string_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_string;
}

nj_object_t *nj_get_null_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_null;
}

nj_object_t *nj_get_function_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_function;
}

nj_object_t *nj_get_cfunction_type_object(nj_state_t *state)
{
	return (nj_object_t*) &state->type_object_cfunction;
}

nj_object_t *nj_get_null_object(nj_state_t *state)
{
	return (nj_object_t*) &state->null_object;
}

nj_object_t *nj_get_true_object(nj_state_t *state)
{
	return (nj_object_t*) &state->true_object;
}

nj_object_t *nj_get_false_object(nj_state_t *state)
{
	return (nj_object_t*) &state->false_object;
}

int nj_object_to_c_int(nj_state_t *state, nj_object_t *object, int64_t *value)
{

	if(object->type == nj_get_int_type_object(state)) {

		if(value)
			*value = ((nj_object_int_t*) object)->value;
	
		return 1;
	}


	if(object->type == nj_get_float_type_object(state)) {

		if(value)
			*value = ((nj_object_float_t*) object)->value;

		return 1;
	}

	return 0;
}

int nj_object_to_c_float(nj_state_t *state, nj_object_t *object, double *value)
{

	if(object->type == nj_get_int_type_object(state)) {

		if(value)
			*value = ((nj_object_int_t*) object)->value;
	
		return 1;
	}


	if(object->type == nj_get_float_type_object(state)) {

		if(value)
			*value = ((nj_object_float_t*) object)->value;

		return 1;
	}

	return 0;
}

int nj_object_to_c_string(nj_state_t *state, nj_object_t *object, const char **value, int *length)
{

	if(object->type == nj_get_string_type_object(state)) {

		if(value)
			*value = ((nj_object_string_t*) object)->value;
	
		if(length)
			*length = ((nj_object_string_t*) object)->length;
	
		return 1;
	}

	return 0;
}

nj_object_t *nj_object_from_c_int(nj_state_t *state, int64_t value)
{
	nj_object_t *o = nj_object_istanciate(state, (nj_object_t*) &state->type_object_int);

	if(o == 0)
		return 0;

	nj_object_int_t *x = (nj_object_int_t*) o;

	x->value = value;

	return o;
}

nj_object_t *nj_object_from_c_float(nj_state_t *state, double value)
{
	nj_object_t *o = nj_object_istanciate(state, (nj_object_t*) &state->type_object_float);

	if(o == 0)
		return 0;

	nj_object_float_t *x = (nj_object_float_t*) o;

	x->value = value;

	return o;
}

nj_object_t *nj_object_from_segment_and_offset(nj_state_t *state, uint32_t segment, uint32_t offset)
{
	nj_object_t *o = nj_object_istanciate(state, (nj_object_t*) &state->type_object_function);

	if(o == 0)
		return 0;

	nj_object_function_t *x = (nj_object_function_t*) o;

	x->segment = segment;
	x->offset = offset;

	return o;
}

nj_object_t *nj_object_type(nj_object_t *self)
{
	return self->type;
}

nj_object_t *nj_object_select(nj_state_t *state, nj_object_t *self, nj_object_t *key)
{

	nj_object_type_t *type = (nj_object_type_t*) self->type;
	
	if(type->on_select == 0)
		return 0;

	return type->on_select(state, self, key);
}

int nj_object_insert(nj_state_t *state, nj_object_t *self, nj_object_t *key, nj_object_t *item)
{
	nj_object_type_t *type = (nj_object_type_t*) self->type;

	if(type->on_insert == 0)
		return 0;

	return type->on_insert(state, self, key, item);
}

nj_object_t *nj_object_select_attribute(nj_state_t *state, nj_object_t *self, const char *name)
{
	nj_object_type_t *type = (nj_object_type_t*) self->type;

	if(type->methods == 0)
		return 0;

	return nj_dictionary_select(state, type->methods, name);
}

int nj_object_insert_attribute(nj_state_t *state, nj_object_t *self, const char *name, nj_object_t *value)
{
	nj_object_type_t *type = (nj_object_type_t*) self->type;

	if(type->methods == 0) {

		nj_object_t *dict = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

		if(dict == 0)
			return 0;
		
		type->methods = dict;
	}

	return nj_dictionary_insert(state, type->methods, name, value);
}



uint8_t nj_object_test(nj_state_t *state, nj_object_t *object)
{
	nj_object_type_t *type = (nj_object_type_t*) object->type;

	if(type->on_test)
		return type->on_test(state, object);

	return 0;
}

nj_object_t *nj_object_istanciate(nj_state_t *state, nj_object_t *type)
{

	size_t object_size = ((nj_object_type_t*) type)->size;
	
	//
	// Align the heap pointer
	//

	if(state->heap_used & 7)
		state->heap_used = (state->heap_used & ~7) + 8;

	//
	// Determine if there is space in the heap or
	// this is an "overflowing allocation"
	//

	nj_object_t *object;

	if(state->heap_used + object_size > state->heap_size) {

		overflow_allocation_t *allocation = malloc(sizeof(overflow_allocation_t) + object_size);

		if(allocation == 0)
			return 0;

		allocation->prev = state->overflow_allocations;
		state->overflow_allocations = allocation;
		
		object = (nj_object_t*) allocation->body;
	
	} else {

		object = (nj_object_t*) (state->heap + state->heap_used);

		state->heap_used += object_size;
	}
	
	//
	// Initialize the object
	//

	memset(object, 0, object_size);

	object->type = type;
	object->flags = OBJECT_IS_COLLECTABLE;

	if(((nj_object_type_t*) type)->on_init)
		((nj_object_type_t*) type)->on_init(state, object);

	return object;
}

void nj_object_print(nj_state_t *state, nj_object_t *self, FILE *fp)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_print) {

		t->on_print(state, self, fp);
	
	} else {

		fprintf(fp, "<Unprintable object>");
	}
}

nj_object_t *nj_object_add(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_add == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_add(state, self, right);
}

nj_object_t *nj_object_sub(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_sub == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_sub(state, self, right);
}

nj_object_t *nj_object_mul(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_mul == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_mul(state, self, right);
}

nj_object_t *nj_object_div(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_div == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_div(state, self, right);
}

nj_object_t *nj_object_mod(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_mod == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_mod(state, self, right);
}

nj_object_t *nj_object_pow(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_pow == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_pow(state, self, right);
}


nj_object_t *nj_object_lss(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_lss == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_lss(state, self, right);
}

nj_object_t *nj_object_grt(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_grt == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_grt(state, self, right);
}

nj_object_t *nj_object_leq(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_leq == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_leq(state, self, right);
}

nj_object_t *nj_object_geq(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_geq == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_geq(state, self, right);
}

nj_object_t *nj_object_eql(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_eql == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_eql(state, self, right);
}

nj_object_t *nj_object_nql(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_nql == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_nql(state, self, right);
}

nj_object_t *nj_object_and(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_and == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_and(state, self, right);
}

nj_object_t *nj_object_or(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_or == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_or(state, self, right);
}

nj_object_t *nj_object_bitwise_and(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_bitwise_and == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_bitwise_and(state, self, right);
}

nj_object_t *nj_object_bitwise_or(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_bitwise_or == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_bitwise_or(state, self, right);
}

nj_object_t *nj_object_bitwise_xor(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_bitwise_xor == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_bitwise_xor(state, self, right);
}

nj_object_t *nj_object_shl(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_shl == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_shl(state, self, right);
}

nj_object_t *nj_object_shr(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_type_t *t = (nj_object_type_t*) self->type;

	if(t->on_shr == 0) {

		// #ERROR
		// Undefined operation
		return 0;
	}

	return t->on_shr(state, self, right);
}