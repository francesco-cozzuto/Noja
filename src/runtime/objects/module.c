
#include "../noja.h"

/*
static nj_object_t *module_select_attribute(nj_state_t *state, nj_object_t *self, const char *name)
{
	(void) state;

	nj_object_module_t *x = (nj_object_module_t*) self;

	return nj_dictionary_select(state, x->dict, name);
}

static int module_insert_attribute(nj_state_t *state, nj_object_t *self, const char *name, nj_object_t *value)
{
	(void) state;

	nj_object_module_t *x = (nj_object_module_t*) self;

	return nj_dictionary_insert(state, x->dict, name, value);
}
*/

int module_methods_setup(nj_state_t *state)
{
(void) state;
	return 1;
}

static int init(nj_state_t *state, nj_object_t *self)
{
	(void) state;

	nj_object_module_t *x = (nj_object_module_t*) self;

	x->dict = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

	if(x->dict == 0)
		return 0;

	return 1;
}

int module_setup(nj_state_t *state)
{

	state->type_object_null = (nj_object_type_t) {

		.super = (nj_object_t) { .new_location = 0, .type = (nj_object_t*) &state->type_object_type, .flags = 0 },
		.name = "Module",
		.size = sizeof(nj_object_module_t),
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