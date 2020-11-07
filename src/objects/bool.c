
#include "../noja.h"

static void bool_print(state_t *state, object_t *self, FILE *fp)
{
	(void) state;

	object_bool_t *x = (object_bool_t*) self;
	
	fprintf(fp, "%s", x->value ? "true" : "false");
}

static uint8_t bool_test(state_t *state, object_t *self)
{
	(void) state;

	object_bool_t *x = (object_bool_t*) self;

	return x->value;
}

int bool_methods_setup(state_t *state)
{
	state->type_object_bool.methods = object_istanciate(state, (object_t*) &state->type_object_dict);

	assert(state->type_object_bool.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!dict_cinsert(state, state->type_object_bool.methods, method_names[i], o))
	
			return 0;
	}

	return 1;
}

int bool_setup(state_t *state)
{
	state->true_object = (object_bool_t) {
		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_bool, .flags = 0 },
		.value = 1,
	};

	state->false_object = (object_bool_t) {
		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_bool, .flags = 0 },
		.value = 0,
	};

	state->type_object_bool = (object_type_t) {

		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_type, .flags = 0 },
		.name = "Bool",
		.size = sizeof(object_bool_t),
		.methods = 0, // Must be created
		.on_init = 0,
		.on_deinit = 0,
		.on_select = 0,
		.on_insert = 0,
		.on_select_attribute = 0,
		.on_insert_attribute = 0,
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