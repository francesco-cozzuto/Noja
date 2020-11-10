
#include <string.h>
#include "builtins.h"

typedef nj_object_t *(*builtin_interface_t)(nj_state_t *state, int argc, nj_object_t **argv);

static nj_object_t *builtin_print(nj_state_t *state, int argc, nj_object_t **argv)
{
	(void) state;

	for(int i = 0; i < argc; i++)

		nj_object_print(state, argv[i], stdout);
	
	printf("\n");

	return (nj_object_t*) &state->null_object;
}

static nj_object_t *builtin_typeof(nj_state_t *state, int argc, nj_object_t **argv)
{
	(void) state;

	if(argc != 1)

		// #ERROR
		// Unexpected arguments 
		return 0;

	return argv[0]->type;
}

static nj_object_t *builtin_typenameof(nj_state_t *state, int argc, nj_object_t **argv)
{
	(void) state;

	if(argc != 1)

		// #ERROR
		// Unexpected arguments 
		return 0;

	const char *name = ((nj_object_type_t*) argv[0]->type)->name;

	return nj_object_from_c_string_ref(state, name, strlen(name));
}

static char *builtin_names[] = {
	"print",
	"type_of",
	"typename_of",
	"disassemble",
};

static builtin_interface_t builtin_routines[] = {
	builtin_print,
	builtin_typeof,
	builtin_typenameof,
	builtin_disassemble,
};

static int builtin_count = sizeof(builtin_names) / sizeof(char*);

int insert_builtins(nj_state_t *state, nj_object_t *dest)
{
	for(int i = 0; i < builtin_count; i++) {

		nj_object_t *o = nj_object_from_c_function(state, builtin_routines[i]);

		if(o == 0)
			return 0;

		if(!nj_dictionary_insert(state, dest, builtin_names[i], o))
	
			return 0;
	}

	return 1;
}