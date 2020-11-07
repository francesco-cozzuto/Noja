

#include <string.h>
#include <stdlib.h>
#include "../noja.h"

enum {
	STRING_IS_OWNED = 1,
};

static int string_init(state_t *state, object_t *self);
static int string_deinit(state_t *state, object_t *self);
static void string_print(state_t *state, object_t *self, FILE *fp);

object_t *object_from_cstring_ref(state_t *state, const char *value, size_t length)
{
	object_t *o = object_istanciate(state, (object_t*) &state->type_object_string);

	if(o == 0)
		return 0;

	object_string_t *x = (object_string_t*) o;

	x->flags = 0;
	x->ref_value = value;
	x->length = length;

	return o;
}

object_t *object_from_cstring(state_t *state, char *value, size_t length)
{
	object_t *o = object_istanciate(state, (object_t*) &state->type_object_string);

	if(o == 0)
		return 0;

	object_string_t *x = (object_string_t*) o;

	x->flags = STRING_IS_OWNED;
	x->value = malloc(length + 1);
	x->length = length;

	if(x->value == 0)
		return 0;

	memcpy(x->value, value, length);
	x->value[length] = '\0';

	return o;
}

static int string_init(state_t *state, object_t *self)
{
	(void) state;

	object_string_t *x = (object_string_t*) self;

	x->flags = 0;
	x->value = 0;
	x->length = 0;
	return 1;
}

static int string_deinit(state_t *state, object_t *self)
{
	(void) state;

	object_string_t *x = (object_string_t*) self;

	if(x->flags & STRING_IS_OWNED)
		free(x->value);
	return 1;	
}

static void string_print(state_t *state, object_t *self, FILE *fp)
{
	(void) state;

	object_string_t *x = (object_string_t*) self;

	fprintf(fp, "%s", x->value);
}

int string_methods_setup(state_t *state)
{
(void) state;
	/*
	state->type_object_string.methods = object_istanciate(state, (object_t*) &state->type_object_dict);

	assert(state->type_object_string.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!dict_cinsert(state, state->type_object_string.methods, method_names[i], o))
	
			return 0;
	}
	*/

	return 1;
}

int string_setup(state_t *state)
{
	state->type_object_string = (object_type_t) {

		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_type, .flags = 0 },
		.name = "String",
		.size = sizeof(object_string_t),
		.methods = 0, // Must be created
		.on_init = string_init,
		.on_deinit = string_deinit,
		.on_select = 0,
		.on_insert = 0,
		.on_select_attribute = 0,
		.on_insert_attribute = 0,
		.on_print = string_print,
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