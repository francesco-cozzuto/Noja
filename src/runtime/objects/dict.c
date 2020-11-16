
#include <string.h>
#include <stdlib.h>
#include "../noja.h"

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

static int dict_init(nj_state_t *state, nj_object_t *self)
{
	(void) state;

	nj_object_dict_t *x = (nj_object_dict_t*) self;

	x->map = malloc(sizeof(int) * 8);
	x->map_size = 8;

	if(x->map == 0)
		return 0;

	for(int i = 0; i < 8; i++)
		x->map[i] = -1;

	x->item_keys   = malloc((sizeof(char*) + sizeof(nj_object_t*)) * 8);
	x->item_values = (nj_object_t**) (x->item_keys + 8);
	x->item_used = 0;
	x->item_size = 8;

	if(x->item_keys == 0) {

		free(x->map);
		return 0;
	}

	return 1;
}

static int dict_deinit(nj_state_t *state, nj_object_t *self)
{
	(void) state;

	nj_object_dict_t *x = (nj_object_dict_t*) self;

	for(int i = 0; i < x->item_used; i++)
		free(x->item_keys[i]);

	free(x->map);
	free(x->item_keys);

	return 1;
}

static void dict_print(nj_state_t *state, nj_object_t *self, FILE *fp)
{
	nj_object_dict_t *x = (nj_object_dict_t*) self;

	(void) state;

	fprintf(fp, "{");

	for(int i = 0; i < x->item_used; i++) {

		fprintf(fp, "\"%s\": ", x->item_keys[i]);
		nj_object_print(state, x->item_values[i], fp);

		if(i+1 < x->item_used)
			fprintf(fp, ", ");
	}

	fprintf(fp, "}");
}

nj_object_t *nj_dictionary_select(nj_state_t *state, nj_object_t *self, const char *name)
{
	(void) state;

	nj_object_dict_t *d = (nj_object_dict_t*) self;

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

int nj_dictionary_merge_in(nj_state_t *state, nj_object_t *self, nj_object_t *other)
{
	nj_object_dict_t *y = (nj_object_dict_t*) other;

	for(int i = 0; i < y->item_used; i++)
		if(!nj_dictionary_insert(state, self, y->item_keys[i], y->item_values[i]))
			return 0;

	return 1;
}

int nj_dictionary_insert(nj_state_t *state, nj_object_t *self, const char *key, nj_object_t *value)
{
	(void) state;

	nj_object_dict_t *d = (nj_object_dict_t*) self;

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

		char *chunk = malloc((sizeof(char*) + sizeof(nj_object_t*)) * d->item_size * 2);

		if(chunk == 0)
			return 0;

		char 	 **new_keys = (char**) chunk;
		nj_object_t **new_values = (nj_object_t**) (chunk + sizeof(char*) * d->item_size * 2);


		for(int i = 0; i < d->item_used; i++) {
	
			new_keys[i] = d->item_keys[i];
			new_values[i] = d->item_values[i];
		}

		
		free(d->item_keys);

		
		d->item_keys   = new_keys;
		d->item_values = new_values;

		d->item_size *= 2;
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

static nj_object_t *dict_select(nj_state_t *state, nj_object_t *self, nj_object_t *key)
{
	(void) state;

	if(key->type != (nj_object_t*) &state->type_object_string) {

		// #ERROR
		// Expected string value as dict key
		return 0;
	}

	return nj_dictionary_select(state, self, ((nj_object_string_t*) key)->value);
}

static int dict_insert(nj_state_t *state, nj_object_t *self, nj_object_t *key, nj_object_t *value)
{
	(void) state;

	if(key->type != (nj_object_t*) &state->type_object_string) {

		// #ERROR
		// Expected string value as dict key
		return 0;
	}

	return nj_dictionary_insert(state, self, ((nj_object_string_t*) key)->value, value);
}

static nj_object_t *method_keys(nj_state_t *state, int argc, nj_object_t **argv)
{
	if(argc != 1)
		return 0;

	if(argv[0]->type != (nj_object_t*) &state->type_object_dict)
		return 0;

	nj_object_dict_t *x = (nj_object_dict_t*) argv[0];

	nj_object_t *array = nj_object_istanciate(state, (nj_object_t*) &state->type_object_array);	

	if(array == 0)
		return 0;

	for(int i = 0; i < x->item_used; i++) {

		nj_object_t *string = nj_object_from_c_string(state, x->item_keys[i], strlen(x->item_keys[i]));

		if(string == 0)
			return 0;

		if(!nj_array_insert(state, array, i, string))
			return 0;
	}

	return array;
}

static nj_object_t *method_length(nj_state_t *state, int argc, nj_object_t **argv)
{
	if(argc != 1)
		return 0;

	if(argv[0]->type != (nj_object_t*) &state->type_object_dict)
		return 0;

	nj_object_dict_t *x = (nj_object_dict_t*) argv[0];

	return nj_object_from_c_int(state, x->item_used);
}	

int dict_methods_setup(nj_state_t *state)
{
(void) state;
	state->type_object_dict.methods = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

	assert(state->type_object_dict.methods);

	static const char *method_names[] = {"keys", "length"};
	static const builtin_interface_t method_routines[] = {method_keys, method_length};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		nj_object_t *o = nj_object_from_c_function(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!nj_dictionary_insert(state, state->type_object_dict.methods, method_names[i], o))
	
			return 0;
	}

	return 1;
}

int dict_setup(nj_state_t *state)
{
	state->type_object_dict = (nj_object_type_t) {

		.super = (nj_object_t) { .type = (nj_object_t*) &state->type_object_type, .flags = 0 },
		.name = "Dict",
		.size = sizeof(nj_object_dict_t),
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