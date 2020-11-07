
#include "../noja.h"

static object_t *module_select_attribute(state_t *state, object_t *self, const char *name)
{
	(void) state;

	object_module_t *x = (object_module_t*) self;

	return dict_cselect(state, x->dict, name);
}

static int module_insert_attribute(state_t *state, object_t *self, const char *name, object_t *value)
{
	(void) state;

	object_module_t *x = (object_module_t*) self;

	return dict_cinsert(state, x->dict, name, value);
}

int module_methods_setup(state_t *state)
{
	return 1;
}

static int init(state_t *state, object_t *self)
{
	(void) state;

	object_module_t *x = (object_module_t*) self;

	x->dict = object_istanciate(state, (object_t*) &state->type_object_dict);

	if(x->dict == 0)
		return 0;

	return 1;
}

int module_setup(state_t *state)
{

	state->type_object_null = (object_type_t) {

		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_type, .flags = 0 },
		.name = "Module",
		.size = sizeof(object_module_t),
		.methods = 0, // Must be created
		.on_init = init,
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