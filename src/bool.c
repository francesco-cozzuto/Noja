
#include "noja.h"

static void bool_print(state_t *state, object_t *self, FILE *fp);

object_type_t bool_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
	.name = "Bool",
	.size = sizeof(object_bool_t),
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
};

object_bool_t object_true = {
	.super = (object_t) { .new_location = 0, .type = (object_t*) &bool_type_object, .flags = 0 },
	.value = 1,
};

object_bool_t object_false = {
	.super = (object_t) { .new_location = 0, .type = (object_t*) &bool_type_object, .flags = 0 },
	.value = 0,
};

static void bool_print(state_t *state, object_t *self, FILE *fp)
{
	object_bool_t *x = (object_bool_t*) self;

	(void) state;
	
	fprintf(fp, "%s", x->value ? "true" : "false");
}