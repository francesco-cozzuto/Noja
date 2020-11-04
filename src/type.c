
#include "noja.h"

object_type_t type_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
	.name = "Type",
	.size = sizeof(object_type_t),
	.methods = 0, // Must be created
	.on_init = 0,
	.on_deinit = 0,
	.on_select = 0,
	.on_insert = 0,
	.on_print = 0,
	.on_add = 0,
	.on_sub = 0,
	.on_mul = 0,
	.on_div = 0,
	.on_test = 0,
};