
#include "../noja.h"

//static object_t *type_select_attribute(state_t *state, object_t *self, const char *name);
//static int type_insert_attribute(state_t *state, object_t *self, const char *name, object_t *value);

object_type_t type_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
	.name = "Type",
	.size = sizeof(object_type_t),
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

/*
static object_t *type_select_attribute(state_t *state, object_t *self, const char *name)
{
	(void) state;

	object_type_t *x = (object_type_t*) self;

	return dict_cselect(state, x->methods, name);
}

static int type_insert_attribute(state_t *state, object_t *self, const char *name, object_t *value)
{
	(void) state;

	object_type_t *x = (object_type_t*) self;

	return dict_cselect(state, x->methods, name, value);
}
*/