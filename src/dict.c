
#include <string.h>
#include <stdlib.h>
#include "noja.h"

static int dict_init(state_t *state, object_t *self);
static int dict_deinit(state_t *state, object_t *self);
static object_t *dict_select(state_t *state, object_t *self, object_t *key);
static int dict_insert(state_t *state, object_t *self, object_t *key, object_t *value);
static void dict_print(state_t *state, object_t *self, FILE *fp);

object_type_t dict_type_object = {

	.super = (object_t) { .new_location = 0, .type = (object_t*) &type_type_object, .flags = 0 },
	.name = "Dict",
	.size = sizeof(object_dict_t),
	.methods = 0, // Must be created
	.on_init = dict_init,
	.on_deinit = dict_deinit,
	.on_select = dict_select,
	.on_insert = dict_insert,
	.on_print = dict_print,
	.on_add = 0,
	.on_sub = 0,
	.on_mul = 0,
	.on_div = 0,
	.on_test = 0,
};


static int hash(const char *key) {

	(void) key;
	return 0;
}

static void map_insert(int *map, int map_size, const char *name, int index) {

	int x, p, i, mask;

	x = hash(name);
	p = x;

	mask = map_size - 1;

	i = x & mask;

	while(1) {

		if(map[i] == -1)
			break;

		p >>= 5;
		i = (i*5 + p + 1) & mask;
	}

	map[i] = index;
}

static int dict_init(state_t *state, object_t *self)
{
	(void) state;

	object_dict_t *x = (object_dict_t*) self;

	x->map = malloc(sizeof(int) * 8);
	x->map_size = 8;

	if(x->map == 0)
		return 0;

	for(int i = 0; i < 8; i++)
		x->map[i] = -1;

	x->item_keys   = malloc((sizeof(char*) + sizeof(object_t*)) * 8);
	x->item_values = (object_t**) (x->item_keys + 8);
	x->item_used = 0;
	x->item_size = 8;

	if(x->item_keys == 0) {

		free(x->map);
		return 0;
	}

	return 1;
}

static int dict_deinit(state_t *state, object_t *self)
{
	(void) state;

	object_dict_t *x = (object_dict_t*) self;

	for(int i = 0; i < x->item_used; i++)
		free(x->item_keys[i]);

	free(x->map);
	free(x->item_keys);

	return 1;
}

static void dict_print(state_t *state, object_t *self, FILE *fp)
{
	object_dict_t *x = (object_dict_t*) self;

	(void) state;

	fprintf(fp, "{");

	for(int i = 0; i < x->item_used; i++) {

		fprintf(fp, "\"%s\": ", x->item_keys[i]);
		object_print(state, x->item_values[i], fp);

		if(i+1 < x->item_used)
			fprintf(fp, ", ");
	}

	fprintf(fp, "}");
}

object_t *dict_cselect(state_t *state, object_t *self, const char *name)
{
	(void) state;

	object_dict_t *d = (object_dict_t*) self;

	int i, x, p, mask;

	x = hash(name);
	p = x;

	mask = d->map_size - 1;

	i = x & mask;

	while(1) {

		if(d->map[i] == -1)
			return 0;

		if(!strcmp(d->item_keys[d->map[i]], name))
				return d->item_values[d->map[i]];

		p >>= 5;
		i = (i*5 + p + 1) & mask;
	}

	return 0;
}


int dict_cinsert(state_t *state, object_t *self, const char *key, object_t *value)
{
	(void) state;

	object_dict_t *d = (object_dict_t*) self;

	// Check if the key was already inserted

	{
		int x, p, i, mask;

		mask = d->map_size - 1;

		x = hash(key);
		p = x;

		i = x & mask;

		while(1) {

			if(d->map[i] == -1)

				// The item isn't here
				break;

			if(!strcmp(d->item_keys[d->map[i]], key)) {

				// Found the item! It's already contained!

				d->item_values[d->map[i]] = value;
				return 1;
			}

			p >>= 5;
			i = (i*5 + p + 1) & mask;
		}
	}

	// ensure there is enough space for the field

	if(d->item_used == d->item_size) {

		char *chunk = malloc((sizeof(char*) + sizeof(object_t*)) * d->item_size * 2);

		if(chunk == 0)
			return 0;

		char 	 **new_keys = (char**) chunk;
		object_t **new_values = (object_t**) (chunk + sizeof(char*) * d->item_size * 2);


		for(int i = 0; i < d->item_used; i++) {
	
			new_keys[i] = d->item_keys[i];
			new_values[i] = d->item_values[i];
		}

		
		free(d->item_keys);

		
		d->item_keys   = new_keys;
		d->item_values = new_values;

		d->item_used *= 2;
	}

	
	if(d->map_size * 2 < d->item_used * 3) {

		
		int  new_map_size = d->map_size * 2;
		int *new_map = malloc(sizeof(int) * new_map_size);
	
		if(new_map == 0)
			return 0;
		
		for(int i = 0; i < new_map_size; i++)
			new_map[i] = -1;

		
		for(int i = 0; i < d->item_used; i++)
			map_insert(new_map, new_map_size, d->item_keys[i], i);

		
		free(d->map);

		
		d->map 	 = new_map;
		d->map_size = new_map_size;
	
	}

	char *key_copy = malloc(strlen(key)+1);

	if(key_copy == 0)
		return 0;

	strcpy(key_copy, key);
	
	// insert the value
	
	map_insert(d->map, d->map_size, key_copy, d->item_used);

	d->item_keys[d->item_used] = key_copy;
	d->item_values[d->item_used] = value;	
	d->item_used++;

	return 1;
}

static object_t *dict_select(state_t *state, object_t *self, object_t *key)
{
	(void) state;

	if(key->type != (object_t*) &string_type_object) {

		// #ERROR
		// Expected string value as dict key
		return 0;
	}

	return dict_cselect(state, self, ((object_string_t*) key)->value);
}

static int dict_insert(state_t *state, object_t *self, object_t *key, object_t *value)
{
	(void) state;

	if(key->type != (object_t*) &string_type_object) {

		// #ERROR
		// Expected string value as dict key
		return 0;
	}

	return dict_cinsert(state, self, ((object_string_t*) key)->value, value);
}
