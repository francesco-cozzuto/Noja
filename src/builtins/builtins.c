
#include <string.h>
#include "builtins.h"

typedef object_t *(*builtin_interface_t)(state_t *state, int argc, object_t **argv);

static object_t *builtin_print(state_t *state, int argc, object_t **argv)
{
	(void) state;

	for(int i = 0; i < argc; i++)

		object_print(state, argv[i], stdout);
	
	printf("\n");

	return (object_t*) &state->null_object;
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

	const char *name = ((object_type_t*) argv[0]->type)->name;

	return object_from_cstring_ref(state, name, strlen(name));
}

static char *builtin_names[] = {
	"print",
	"type_of",
	"typename_of",
	"disassemble",
	"import",
};

static builtin_interface_t builtin_routines[] = {
	builtin_print,
	builtin_typeof,
	builtin_typenameof,
	builtin_disassemble,
	builtin_import,
};

static int builtin_count = sizeof(builtin_names) / sizeof(char*);

int insert_builtins(state_t *state, object_t *dest)
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