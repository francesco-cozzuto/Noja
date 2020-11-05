
#include <stdlib.h>
#include <string.h>
#include "../noja.h"

static int array_init(state_t *state, object_t *self);
static int array_deinit(state_t *state, object_t *self);
static void array_print(state_t *state, object_t *self, FILE *fp);
static object_t *array_select(state_t *state, object_t *self, object_t *key);
static int array_insert(state_t *state, object_t *self, object_t *key, object_t *value);

object_type_t array_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
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
	.on_test = 0,
};

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

	if(key->type != (object_t*) &int_type_object) {

		// #ERROR
		// Expected an int value as array key
		return 0;
	}

	return array_cselect(state, self, ((object_int_t*) key)->value);
}

static int array_insert(state_t *state, object_t *self, object_t *key, object_t *value)
{

	if(key->type != (object_t*) &int_type_object) {

		// #ERROR
		// Expected int value as array key
		return 0;
	}

	return array_cinsert(state, self, ((object_int_t*) key)->value, value);
}
