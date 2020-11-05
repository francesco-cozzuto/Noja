

#include "noja.h"

static void float_print(state_t *state, object_t *self, FILE *fp);
static object_t *float_add(state_t *state, object_t *self, object_t *right);
static uint8_t float_test(state_t *state, object_t *self);

object_type_t float_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
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
	.on_test = float_test,
};

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

	if(right->type != (object_t*) &float_type_object) {

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