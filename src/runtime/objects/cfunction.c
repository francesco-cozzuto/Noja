

#include "../noja.h"

nj_object_t *nj_object_from_c_function(nj_state_t *state, nj_object_t *(*routine)(nj_state_t *state, int argc, nj_object_t **argv))
{
	nj_object_t *o = nj_object_istanciate(state, (nj_object_t*) &state->type_object_cfunction);

	if(o == 0)
		return 0;

	nj_object_cfunction_t *x = (nj_object_cfunction_t*) o;

	x->routine = routine;

	return o;
}

int cfunction_methods_setup(nj_state_t *state)
{
(void) state;
	/*
	state->type_object_cfunction.methods = object_istanciate(state, (nj_object_t*) &state->type_object_dict);

	assert(state->type_object_cfunction.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		nj_object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!nj_dictionary_insert(state, state->type_object_cfunction.methods, method_names[i], o))
	
			return 0;
	}
	*/

	return 1;
}

int cfunction_setup(nj_state_t *state)
{
	state->type_object_cfunction = (nj_object_type_t) {

		.super = (nj_object_t) { .new_location = 0, .type = (nj_object_t*) &state->type_object_type, .flags = 0 },
		.name = "CFunction",
		.size = sizeof(nj_object_cfunction_t),
		.methods = 0, // Must be created
		.on_init = 0,
		.on_deinit = 0,
		.on_select = 0,
		.on_insert = 0,
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