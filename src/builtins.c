
#include <string.h>
#include "noja.h"

typedef object_t *(*builtin_interface_t)(state_t *state, int argc, object_t **argv);

static object_t *builtin_print(state_t *state, int argc, object_t **argv)
{
	(void) state;

	for(int i = 0; i < argc; i++)

		object_print(state, argv[i], stdout);
	
	printf("\n");

	return (object_t*) &object_null;
}

static object_t *builtin_typeof(state_t *state, int argc, object_t **argv)
{
	(void) state;

	if(argc != 1)

		// #ERROR
		// Unexpected arguments 
		return 0;

	return argv[0]->type;
}

static object_t *builtin_typenameof(state_t *state, int argc, object_t **argv)
{
	(void) state;

	if(argc != 1)

		// #ERROR
		// Unexpected arguments 
		return 0;

	char *name = ((object_type_t*) argv[0]->type)->name;

	return object_from_cstring_ref(state, name, strlen(name));
}

static char *builtin_names[] = {
	"print",
	"type_of",
	"typename_of",
};

static builtin_interface_t builtin_routines[] = {
	builtin_print,
	builtin_typeof,
	builtin_typenameof,
};

static int builtin_count = sizeof(builtin_names) / sizeof(char*);

int insert_builtins(state_t *state, object_t *dest, char *error_buffer, int error_buffer_size)
{
	for(int i = 0; i < builtin_count; i++) {

		object_t *o = object_from_cfunction(state, builtin_routines[i]);

		if(o == 0)
			return 0;

		if(!dict_cinsert(state, dest, builtin_names[i], o))
	
			return 0;
	}

	return 1;
}