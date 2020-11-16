
#include "../noja.h"

static void bool_print(nj_state_t *state, nj_object_t *self, FILE *fp)
{
	(void) state;

	nj_object_bool_t *x = (nj_object_bool_t*) self;
	
	fprintf(fp, "%s", x->value ? "true" : "false");
}

static uint8_t bool_test(nj_state_t *state, nj_object_t *self)
{
	(void) state;

	nj_object_bool_t *x = (nj_object_bool_t*) self;

	return x->value;
}

int bool_methods_setup(nj_state_t *state)
{
(void) state;
	/*
	state->type_object_bool.methods = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

	assert(state->type_object_bool.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		nj_object_t *o = nj_object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!nj_dictionary_insert(state, state->type_object_bool.methods, method_names[i], o))
	
			return 0;
	}
	*/
	return 1;
}

int bool_setup(nj_state_t *state)
{
	state->true_object = (nj_object_bool_t) {
		.super = (nj_object_t) { .new_location = 0, .type = (nj_object_t*) &state->type_object_bool, .flags = 0 },
		.value = 1,
	};

	state->false_object = (nj_object_bool_t) {
		.super = (nj_object_t) { .new_location = 0, .type = (nj_object_t*) &state->type_object_bool, .flags = 0 },
		.value = 0,
	};

	state->type_object_bool = (nj_object_type_t) {

		.super = (nj_object_t) { .new_location = 0, .type = (nj_object_t*) &state->type_object_type, .flags = 0 },
		.name = "Bool",
		.size = sizeof(nj_object_bool_t),
		.methods = 0, // Must be created
		.on_init = 0,
		.on_deinit = 0,
		.on_select = 0,
		.on_insert = 0,
		.on_print = bool_print,
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
		.on_test = bool_test,
	};

	return 1;
}