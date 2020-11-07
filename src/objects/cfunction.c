

#include "../noja.h"

object_t *object_from_cfunction(state_t *state, object_t *(*routine)(state_t *state, int argc, object_t **argv))
{
	object_t *o = object_istanciate(state, (object_t*) &state->type_object_cfunction);

	if(o == 0)
		return 0;

	object_cfunction_t *x = (object_cfunction_t*) o;

	x->routine = routine;

	return o;
}

int cfunction_methods_setup(state_t *state)
{
	state->type_object_cfunction.methods = object_istanciate(state, (object_t*) &state->type_object_dict);

	assert(state->type_object_cfunction.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!dict_cinsert(state, state->type_object_cfunction.methods, method_names[i], o))
	
			return 0;
	}

	return 1;
}

int cfunction_setup(state_t *state)
{
	state->type_object_cfunction = (object_type_t) {

		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_type, .flags = 0 },
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
		.on_mod = 0,
		.on_pow = 0,
		.on_lss = 0,
		.on_grt = 0,
		.on_leq = 0,
		.on_geq = 0,
		.on_eql = 0,
		.on_nql = 0,
		.on_and = 0,
		.on_or  = 0,
		.on_bitwise_and = 0,
		.on_bitwise_or  = 0,
		.on_bitwise_xor = 0,
		.on_shl = 0,
		.on_shr = 0,
		.on_test = 0,
	};

	return 1;
}