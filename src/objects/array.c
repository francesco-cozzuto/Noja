
#include <stdlib.h>
#include <string.h>
#include "../noja.h"

static int array_init(state_t *state, object_t *self)
{
	(void) state;

	object_array_t *x = (object_array_t*) self;

	x->items = malloc(sizeof(object_t*) * 8);
	x->item_used = 0;
	x->item_size = 8;

	if(x->items == 0)
		return 0;

	return 1;
}

static int array_deinit(state_t *state, object_t *self)
{
	(void) state;

	object_array_t *x = (object_array_t*) self;

	free(x->items);

	return 1;
}

static void array_print(state_t *state, object_t *self, FILE *fp)
{
	object_array_t *x = (object_array_t*) self;

	(void) state;

	fprintf(fp, "[");

	for(int i = 0; i < x->item_used; i++) {

		object_print(state, x->items[i], fp);

		if(i+1 < x->item_used)
			fprintf(fp, ", ");
	}

	fprintf(fp, "]");
}

object_t *array_cselect(state_t *state, object_t *self, int64_t index)
{
	(void) state;

	object_array_t *a = (object_array_t*) self;

	if(index < 0 || index > a->item_used-1)
		return 0;

	return a->items[index];
}

int array_cinsert(state_t *state, object_t *self, int64_t index, object_t *value)
{
	(void) state;

	object_array_t *a = (object_array_t*) self;

	if(index < 0 || index > a->item_used)
		return 0;

	if(index == a->item_used) {

		if(a->item_used == a->item_size) {

			object_t **items = malloc(sizeof(object_t*) * a->item_size * 2);

			memcpy(items, a->items, sizeof(object_t*) * a->item_used);

			free(a->items);

			a->items = items;
			a->item_size *= 2;

		}

		a->items[a->item_used++] = value;

	} else {

		a->items[index] = value;
	}

	return 1;
}

static object_t *array_select(state_t *state, object_t *self, object_t *key)
{

	if(key->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Expected an int value as array key
		return 0;
	}

	return array_cselect(state, self, ((object_int_t*) key)->value);
}

static int array_insert(state_t *state, object_t *self, object_t *key, object_t *value)
{

	if(key->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Expected int value as array key
		return 0;
	}

	return array_cinsert(state, self, ((object_int_t*) key)->value, value);
}

static object_t *method_length(state_t *state, int argc, object_t **argv)
{
	if(argc != 1)
		return 0;

	if(argv[0]->type != (object_t*) &state->type_object_array)
		return 0;

	return object_from_cint(state, ((object_array_t*) argv[0])->item_used);
}

int array_methods_setup(state_t *state)
{
(void) state;
	state->type_object_array.methods = object_istanciate(state, (object_t*) &state->type_object_dict);

	assert(state->type_object_array.methods);

	static const char *method_names[] = {"length"};
	static const builtin_interface_t method_routines[] = { method_length };

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!dict_cinsert(state, state->type_object_array.methods, method_names[i], o))
	
			return 0;
	}

	return 1;
}

int array_setup(state_t *state)
{
	state->type_object_array = (object_type_t) {
		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_type, .flags = 0 },
		.name = "Array",
		.size = sizeof(object_array_t),
		.methods = 0, // Must be created
		.on_init = array_init,
		.on_deinit = array_deinit,
		.on_select = array_select,
		.on_insert = array_insert,
		.on_select_attribute = 0,
		.on_insert_attribute = 0,
		.on_print = array_print,
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