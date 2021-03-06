
#include "../noja.h"

static void float_print(nj_state_t *state, nj_object_t *self, FILE *fp)
{
	(void) state;

	nj_object_float_t *x = (nj_object_float_t*) self;
	
	fprintf(fp, "%g", x->value);
}

static nj_object_t *float_add(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_float_t *x = (nj_object_float_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_float) {

		// #ERROR
		// Unexpected type in add operation
		return 0;
	}

	nj_object_float_t *r = (nj_object_float_t*) right;

	return nj_object_from_c_float(state, x->value + r->value);
}

static uint8_t float_test(nj_state_t *state, nj_object_t *self)
{
	(void) state;
	
	nj_object_float_t *x = (nj_object_float_t*) self;

	return x->value != 0;
}

int float_methods_setup(nj_state_t *state)
{
(void) state;
	/*
	state->type_object_float.methods = object_istanciate(state, (nj_object_t*) &state->type_object_dict);

	assert(state->type_object_float.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		nj_object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!nj_dictionary_insert(state, state->type_object_float.methods, method_names[i], o))
	
			return 0;
	}
	*/

	return 1;
}

int float_setup(nj_state_t *state)
{
	state->type_object_float = (nj_object_type_t) {

		.super = (nj_object_t) { .type = (nj_object_t*) &state->type_object_type, .flags = 0 },
		.name = "Float",
		.size = sizeof(nj_object_float_t),
		.methods = 0, // Must be created
		.on_init = 0,
		.on_deinit = 0,
		.on_select = 0,
		.on_insert = 0,
		.on_print = float_print,
		.on_add = float_add,
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
		.on_test = float_test,
	};

	return 1;
}