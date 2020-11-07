
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "noja.h"
#include "utils/basic.h"

int step(state_t *state);
int insert_builtins(state_t *state, object_t *dest);
static int  state_init(state_t *state, executable_t *executable, string_builder_t *output_builder);
static void state_deinit(state_t *state);

int run_text_inner(const char *text, int length, string_builder_t *output_builder)
{
	executable_t *executable;
	state_t state;
	ast_t ast;

	if(!parse(text, length, &ast, output_builder))
		return 0;

	//ast_print(ast, stdout);

	executable = generate(ast);

	ast_delete(ast);
	assert(executable);

	if(!state_init(&state, executable, output_builder))
		return 0;

	while(step(&state));

	int result = !state.failed;

	state_deinit(&state);

	free(executable);
	return result;	
}

int run_text(const char *text, int length, char **error_text)
{
	string_builder_t output_builder;
	string_builder_init(&output_builder);

	int result = run_text_inner(text, length, &output_builder);

	if(!result && error_text) {

		(*error_text) = malloc(output_builder.length + 1);

		if(*error_text)
			string_builder_serialize_to_buffer(&output_builder, *error_text);
	}

	string_builder_deinit(&output_builder);
	return result;
}

int run_file(const char *path, char **error_text)
{
	char *text;
	int length;

	if(!load_text(path, &text, &length)) {

		// Failed to load file contents
		
		return 0;
	}

	int result = run_text(text, length, error_text);

	free(text);

	return result;
}

int array_setup(state_t *state);
int bool_setup(state_t *state);
int dict_setup(state_t *state);
int int_setup(state_t *state);
int function_setup(state_t *state);
int cfunction_setup(state_t *state);
int null_setup(state_t *state);
int string_setup(state_t *state);
int type_setup(state_t *state);
int float_setup(state_t *state);

int array_methods_setup(state_t *state);
int bool_methods_setup(state_t *state);
int dict_methods_setup(state_t *state);
int int_methods_setup(state_t *state);
int function_methods_setup(state_t *state);
int cfunction_methods_setup(state_t *state);
int null_methods_setup(state_t *state);
int string_methods_setup(state_t *state);
int type_methods_setup(state_t *state);
int float_methods_setup(state_t *state);

static int state_init(state_t *state, executable_t *executable, string_builder_t *output_builder)
{
	state->heap = malloc(4096);

	if(state->heap == 0)
		return 0;

	state->stack = malloc(sizeof(void*) * 128);

	if(state->stack == 0) {

		free(state->heap);
		return 0;
	}

	state->variable_maps = malloc(sizeof(void*) * 128);

	if(state->variable_maps == 0) {

		free(state->heap);
		return 0;
	}

	state->failed = 0;
	state->output_builder = output_builder;

	state->heap_size = 4096;
	state->heap_used = 0;
	state->overflow_allocations = 0;

	state->stack_item_count = 0;
	state->stack_item_count_max = 128;

	state->continue_destinations_depth = 0;
	state->break_destinations_depth = 0;
	state->executable_stack[0] = executable;
	state->program_counters[0] = 0;
	state->call_depth = 1;

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

	state->variable_maps[0] = object_istanciate(state, (object_t*) &state->type_object_dict);
	state->variable_maps_count = 1;
	state->variable_maps_count_max = 128;
	assert(state->variable_maps[0]);

	state->builtins_map = object_istanciate(state, (object_t*) &state->type_object_dict);
	assert(state->builtins_map);

	if(!insert_builtins(state, state->builtins_map))
		return 0;

	state->argc = -1;

	return 1;
}

static void state_deinit(state_t *state)
{
	free(state->heap);
	free(state->stack);
	free(state->variable_maps);
}