
#include "../noja.h"

static void float_print(state_t *state, object_t *self, FILE *fp)
{
	(void) state;

	object_float_t *x = (object_float_t*) self;
	
	fprintf(fp, "%g", x->value);
}

static object_t *float_add(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_float_t *x = (object_float_t*) self;

	if(right->type != (object_t*) &state->type_object_float) {

		// #ERROR
		// Unexpected type in add operation
		return 0;
	}

	object_float_t *r = (object_float_t*) right;

	return object_from_cfloat(state, x->value + r->value);
}

static uint8_t float_test(state_t *state, object_t *self)
{
	(void) state;
	
	object_float_t *x = (object_float_t*) self;

	return x->value != 0;
}

int float_methods_setup(state_t *state)
{
	state->type_object_float.methods = object_istanciate(state, (object_t*) &state->type_object_dict);

	assert(state->type_object_float.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!dict_cinsert(state, state->type_object_float.methods, method_names[i], o))
	
			return 0;
	}

	return 1;
}

int float_setup(state_t *state)
{
	state->type_object_float = (object_type_t) {

		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_type, .flags = 0 },
		.name = "Float",
		.size = sizeof(object_float_t),
		.methods = 0, // Must be created
		.on_init = 0,
		.on_deinit = 0,
		.on_select = 0,
		.on_insert = 0,
		.on_select_attribute = 0,
		.on_insert_attribute = 0,
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