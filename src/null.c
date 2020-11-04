
#include "noja.h"

static void null_print(state_t *state, object_t *self, FILE *fp);

object_type_t null_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
	.name = "Bool",
	.size = sizeof(object_t),
	.methods = 0, // Must be created
	.on_init = 0,
	.on_deinit = 0,
	.on_select = 0,
	.on_insert = 0,
	.on_print = null_print,
	.on_add = 0,
	.on_sub = 0,
	.on_mul = 0,
	.on_div = 0,
	.on_test = 0,
};

object_t object_null = {
	.new_location = 0, 
	.type = (object_t*) &null_type_object, 
	.flags = 0,
};

static void null_print(state_t *state, object_t *self, FILE *fp)
{
	(void) state;
	(void) self;

	fprintf(fp, "null");
}