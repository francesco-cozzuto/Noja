
#include "../noja.h"

static void int_print(state_t *state, object_t *self, FILE *fp);
static object_t *int_add(state_t *state, object_t *self, object_t *right);
static uint8_t int_test(state_t *state, object_t *self);

object_type_t int_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
	.name = "Int",
	.size = sizeof(object_int_t),
	.methods = 0, // Must be created
	.on_init = 0,
	.on_deinit = 0,
	.on_select = 0,
	.on_insert = 0,
	.on_select_attribute = 0,
	.on_insert_attribute = 0,
	.on_print = int_print,
	.on_add = int_add,
	.on_sub = 0,
	.on_mul = 0,
	.on_div = 0,
	.on_test = int_test,
};

static void int_print(state_t *state, object_t *self, FILE *fp)
{
	object_int_t *x = (object_int_t*) self;

	(void) state;
	
	fprintf(fp, "%ld", x->value);
}

static object_t *int_add(state_t *state, object_t *self, object_t *right)
{
	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &int_type_object) {

		// #ERROR
		// Unexpected type in add operation
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value + r->value);
}

static uint8_t int_test(state_t *state, object_t *self)
{
	(void) state;
	
	object_int_t *x = (object_int_t*) self;

	return x->value != 0;
}