
#include "../noja.h"

static void bool_print(state_t *state, object_t *self, FILE *fp);
static uint8_t bool_test(state_t *state, object_t *self);

object_type_t bool_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
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