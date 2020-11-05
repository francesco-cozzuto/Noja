

#include "noja.h"

object_type_t cfunction_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
	.name = "CFunction",
	.size = sizeof(object_cfunction_t),
	.methods = 0, // Must be created
	.on_init = 0,
	.on_deinit = 0,
	.on_select = 0,
	.on_insert = 0,
	.on_select_attribute = 0,
	.on_insert_attribute = 0,
	.on_print = 0,
	.on_add = 0,
	.on_sub = 0,
	.on_mul = 0,
	.on_div = 0,
	.on_test = 0,
};

object_t *object_from_cfunction(state_t *state, object_t *(*routine)(state_t *state, int argc, object_t **argv))
{
	object_t *o = object_istanciate(state, (object_t*) &cfunction_type_object);

	if(o == 0)
		return 0;

	object_cfunction_t *x = (object_cfunction_t*) o;

	x->routine = routine;

	return o;
}