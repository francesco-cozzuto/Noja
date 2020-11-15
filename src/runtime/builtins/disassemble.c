
#include <assert.h>
#include "builtins.h"

nj_object_t *builtin_disassemble(nj_state_t *state, int argc, nj_object_t **argv)
{
	(void) state;
	(void) argv;

	if(argc != 0)

		// #ERROR
		// Unexpected arguments 
		return 0;

	char *code, *data;
	int code_length, data_length;

	code = state->segments[u32_top(&state->segment_stack)].code;
	data = state->segments[u32_top(&state->segment_stack)].data;
	code_length = state->segments[u32_top(&state->segment_stack)].code_size;
	data_length = state->segments[u32_top(&state->segment_stack)].data_size;

	nj_disassemble(code, data, code_length, data_length);

	return (nj_object_t*) &state->null_object;
}