
#include "../noja.h"

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
	.on_select_attribute = 0,
	.on_insert_attribute = 0,
	.on_print = null_print,
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