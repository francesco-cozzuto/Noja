
#include "builtins.h"

object_t *builtin_import(state_t *state, int argc, object_t **argv)
{
	(void) state;

	if(argc != 1)
		return 0;

	

	return (object_t*) &state->null_object;
}
