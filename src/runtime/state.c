
#include <stdarg.h>
#include <stdlib.h>
#include "noja.h"

void nj_fail(nj_state_t *state, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	string_builder_append_p(state->output_builder, fmt, args);
	va_end(args);

	state->failed = 1;
}

int nj_failed(nj_state_t *state)
{
	return state->failed;
}

int insert_builtins(nj_state_t *state, nj_object_t *dest);

int array_setup(nj_state_t *state);
int bool_setup(nj_state_t *state);
int dict_setup(nj_state_t *state);
int int_setup(nj_state_t *state);
int function_setup(nj_state_t *state);
int cfunction_setup(nj_state_t *state);
int null_setup(nj_state_t *state);
int string_setup(nj_state_t *state);
int type_setup(nj_state_t *state);
int float_setup(nj_state_t *state);

int array_methods_setup(nj_state_t *state);
int bool_methods_setup(nj_state_t *state);
int dict_methods_setup(nj_state_t *state);
int int_methods_setup(nj_state_t *state);
int function_methods_setup(nj_state_t *state);
int cfunction_methods_setup(nj_state_t *state);
int null_methods_setup(nj_state_t *state);
int string_methods_setup(nj_state_t *state);
int type_methods_setup(nj_state_t *state);
int float_methods_setup(nj_state_t *state);

int nj_state_init(nj_state_t *state, string_builder_t *output_builder)
{
	state->heap.chunk = malloc(65536);
	state->heap.size = 65536;
	state->heap.used = 0;
	state->heap.overflow_allocations = NULL;

	if(state->heap.chunk == 0)
		return 0;

	state->failed = 0;
	state->output_builder = output_builder;

	object_stack_init(&state->eval_stack);
	object_stack_init(&state->vars_stack);
	u32_stack_init(&state->segment_stack);
	u32_stack_init(&state->offset_stack);

	assert(cfunction_setup(state));
	assert(dict_setup(state));
	assert(array_setup(state));
	assert(bool_setup(state));
	assert(int_setup(state));
	assert(function_setup(state));
	assert(null_setup(state));
	assert(string_setup(state));
	assert(type_setup(state));
	assert(float_setup(state));

	assert(cfunction_methods_setup(state));
	assert(dict_methods_setup(state));
	assert(array_methods_setup(state));
	assert(bool_methods_setup(state));
	assert(int_methods_setup(state));
	assert(function_methods_setup(state));
	assert(null_methods_setup(state));
	assert(string_methods_setup(state));
	assert(type_methods_setup(state));
	assert(float_methods_setup(state));

	state->builtins_map = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);
	assert(state->builtins_map);

	if(!insert_builtins(state, state->builtins_map))
		return 0;

	state->argc = -1;

	state->segments = malloc(sizeof(segment_t) * 4);
	state->segments_size = 4;
	state->segments_used = 0;

	if(state->segments == 0)
		return 0;

	return 1;
}

void nj_state_deinit(nj_state_t *state)
{
	nj_destroy_heap(state, &state->heap);

	for(int i = 0; i < state->segments_used; i++)
		free(state->segments[i].code);

	free(state->segments);

	object_stack_deinit(&state->eval_stack);
	object_stack_deinit(&state->vars_stack);
	u32_stack_deinit(&state->segment_stack);
	u32_stack_deinit(&state->offset_stack);
}
