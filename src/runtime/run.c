
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <dlfcn.h>

#include "noja.h"
#include "utils/basic.h"

int step(nj_state_t *state);
int insert_builtins(nj_state_t *state, nj_object_t *dest);
static int  state_init(nj_state_t *state, string_builder_t *output_builder);
static void state_deinit(nj_state_t *state);

int append_segment(nj_state_t *state, char *code, char *data, uint32_t code_size, uint32_t data_size, ast_t ast, uint32_t *e_segment)
{
	if(state->segments_used == state->segments_size) {

		segment_t *segments = malloc(sizeof(segment_t) * (state->segments_size + 16));

		if(segments == 0)
			return 0;

		memcpy(segments, state->segments, sizeof(segment_t) * state->segments_used);

		free(state->segments);
		state->segments = segments;

		state->segments_size += 16;
	}

	if(e_segment)
		*e_segment = state->segments_used;

	nj_object_t *map = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

	if(map == 0)
		return 0;

	state->segments[state->segments_used] = (segment_t) { 
		.ast = ast,
		.code = code, 
		.data = data, 
		.code_size = code_size, 
		.data_size = data_size,
		.global_variables_map = map,
	};

	state->segments_used++;

	return 1;
}

static int run_text_inner(const char *text, int length, string_builder_t *output_builder)
{
	nj_state_t state;
	ast_t ast;

	if(!parse(text, length, &ast, output_builder))
		return 0;

	//ast_print(ast, stdout);

	char *first_code_segment;
	char *first_data_segment;
	uint32_t first_code_segment_size;
	uint32_t first_data_segment_size;

	if(!generate(ast, &first_data_segment, &first_code_segment, &first_data_segment_size, &first_code_segment_size)) {

		string_builder_append(output_builder, "Failed to generate bytecode");
		ast_delete(ast);
		return 0;
	}

	if(!state_init(&state, output_builder)) {

		ast_delete(ast);
		free(first_code_segment);
		return 0;
	}

	if(!append_segment(&state, first_code_segment, first_data_segment, first_code_segment_size, first_data_segment_size, ast, 0)) {

		ast_delete(ast);
		free(first_code_segment);
		return 0;
	}

	if(!u32_push(&state.segment_stack, 0) || !u32_push(&state.offset_stack, 0)) {

		ast_delete(ast);
		free(first_code_segment);
		return 0;
	}

	while(step(&state));

	u32_pop(&state.segment_stack);
	u32_pop(&state.offset_stack);

	int result = !state.failed;

	state_deinit(&state);
	return result;	
}

int nj_run(const char *text, int length, char **error_text)
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

int nj_run_file(const char *path, char **error_text)
{
	char *text;
	int length;

	if(!load_text(path, &text, &length)) {

		// Failed to load file contents
		
		return 0;
	}

	int result = nj_run(text, length, error_text);

	free(text);

	return result;
}

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

static int state_init(nj_state_t *state, string_builder_t *output_builder)
{
	state->heap = malloc(4096);
	state->heap_size = 4096;
	state->heap_used = 0;
	state->overflow_allocations = 0;

	if(state->heap == 0)
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

static void state_deinit(nj_state_t *state)
{
	free(state->heap); // free overflow allocations!

	for(int i = 0; i < state->segments_used; i++)
		free(state->segments[i].code);

	free(state->segments);

	object_stack_deinit(&state->eval_stack);
	object_stack_deinit(&state->vars_stack);
	u32_stack_deinit(&state->segment_stack);
	u32_stack_deinit(&state->offset_stack);
}

static void fetch_u32(nj_state_t *state, uint32_t *value);
static void fetch_i64(nj_state_t *state, int64_t *value);
static void fetch_f64(nj_state_t *state, double *value);
static void fetch_string(nj_state_t *state, char **value);

static nj_object_t *do_binary_import(nj_state_t *state, char *path)
{
	void *handle = dlopen(path, RTLD_LAZY | RTLD_NODELETE);

	if(handle == 0) {
		nj_fail(state, "Failed to open the library file \"${zero-terminated-string}\" (${zero-terminated-string})", path, dlerror());
		return 0;
	}

	dlerror();

	nj_object_t *(*setup)(nj_state_t *state);

	setup = dlsym(handle, "setup");

	if(setup == 0) {
		nj_fail(state, "Failed to load symbols from \"${zero-terminated-string}\". There was no setup routine! (${zero-terminated-string})", path, dlerror());
		return 0;
	}

	nj_object_t *result = setup(state);

	dlclose(handle);

	if(result == 0 && !nj_failed(state))
		nj_fail(state, "Setup routine of \"${zero-terminated-string}\" returned null but didn't raise an error!", path);

	return result;
}

static nj_object_t *do_text_import(nj_state_t *state, char *path)
{
	char *text;
	ast_t ast;
	char *code_segment;
	char *data_segment;
	uint32_t code_segment_size;
	uint32_t data_segment_size;
	int length;

	//
	// Load the source file text
	//

	if(!load_text(path, &text, &length)) {

		nj_fail(state, "Failed to load \"${zero-terminated-string}\" in memory", path);
		return 0;
	}

	//
	// Parse it
	//


	if(!parse(text, length, &ast, state->output_builder)) {

		state->failed = 1;
		free(text);
		return 0;
	}

	free(text);

	//
	// Generate the bytecode
	//

	if(!generate(ast, &data_segment, &code_segment, &data_segment_size, &code_segment_size)) {

		nj_fail(state, "Failed to generate bytecode for \"${zero-terminated-string}\"", path);
		ast_delete(ast);
		return 0;
	}

	//
	// Create a new segment
	//

	uint32_t imported_segment;

	if(!append_segment(state, code_segment, data_segment, code_segment_size, data_segment_size, ast, &imported_segment)) {

		// #ERROR

		free(code_segment);
		ast_delete(ast);
		nj_fail(state, "Out of memory. Failed to grow segment array");
		return 0;
	}

	//
	// Run the segment
	//

	u32_push(&state->segment_stack, imported_segment);
	u32_push(&state->offset_stack, 0);

	while(step(state));

	if(state->failed)
		return 0;

	u32_pop(&state->segment_stack);
	u32_pop(&state->offset_stack);

	return state->segments[imported_segment].global_variables_map;
}

static nj_object_t *current_map(nj_state_t *state)
{
	if(object_stack_size(&state->vars_stack) == 0)

		return state->segments[u32_top(&state->segment_stack)].global_variables_map;

	return object_top(&state->vars_stack);
}

static void do_named_import(nj_state_t *state)
{
	char *name;

	//
	// Fetch import name
	//

	fetch_string(state, &name);

	if(nj_failed(state))
		return;

	if(object_stack_size(&state->eval_stack) == 0) {

		// #ERROR
		nj_fail(state, "OPCODE_IMPORT_AS on an empty stack");
		return;
	}

	// Pop an object from the evaluation stack
	// and try to get the path from it.
	
	nj_object_t *path_object = object_pop(&state->eval_stack);

	if(path_object->type != (nj_object_t*) &state->type_object_string) {

		// #ERROR
		nj_fail(state, "The imported path expression is not a string");
		return;
	}

	// Get the raw path representation

	char *path = ((nj_object_string_t*) path_object)->value;
	
	// Get the extension

	char *extension = strrchr(path, '.');

    // If there is no extension, or if there is an extension but it's not ".dll" or ".so",
	// we expect a noja text source file, else we expect a shared library.

	nj_object_t *map;

	if(!extension || (strcmp(extension, ".dll") && strcmp(extension, ".so"))) {

		// Handle the import of a noja source

		map = do_text_import(state, path);

	} else {

		// Handle the import of a shared library

		map = do_binary_import(state, path);
	}

	if(nj_failed(state))
		return;

	if(!nj_dictionary_insert(state, current_map(state), name, map)) {

		// #ERROR
		nj_fail(state, "Failed to create imported module variable. Couldn't insert into the variable map");
		return;
	}
}

static void do_unnamed_import(nj_state_t *state)
{

	if(object_stack_size(&state->eval_stack) == 0) {

		// #ERROR
		nj_fail(state, "OPCODE_IMPORT on an empty stack");
		return;
	}

	// Pop an object from the evaluation stack
	// and try to get the path from it.
	
	nj_object_t *path_object = object_pop(&state->eval_stack);

	if(path_object->type != (nj_object_t*) &state->type_object_string) {

		// #ERROR
		nj_fail(state, "The imported path expression is not a string");
		return;
	}

	// Get the raw path representation

	char *path = ((nj_object_string_t*) path_object)->value;
	
	// Get the extension

	char *extension = strrchr(path, '.');

    // If there is no extension, or if there is an extension but it's not ".dll" or ".so",
	// we expect a noja text source file, else we expect a shared library.

	nj_object_t *map;

	if(!extension || (strcmp(extension, ".dll") && strcmp(extension, ".so"))) {

		// Handle the import of a noja source

		map = do_text_import(state, path);

	} else {

		// Handle the import of a shared library

		map = do_binary_import(state, path);
	}

	if(nj_failed(state))
		return;

	//
	// Insert the symbols in it
	//

	if(!nj_dictionary_merge_in(state, current_map(state), map)) {

		// #ERROR
		nj_fail(state, "Failed to create import module symnols");
		return;
	}
}

int step(nj_state_t *state)
{
	uint32_t opcode;

	fetch_u32(state, &opcode);

	if(nj_failed(state)) return 0;

	switch(opcode) {

		case OPCODE_NOPE:
		break;

		case OPCODE_QUIT:
		return 0;

		case OPCODE_IMPORT: 
		do_unnamed_import(state); 
		break;
		
		case OPCODE_IMPORT_AS: 
		do_named_import(state); 
		break;
		

		case OPCODE_PUSH_NULL:

		if(!object_push(&state->eval_stack, (nj_object_t*) &state->null_object)) {
			
			// #ERROR
			nj_fail(state, "Out of memory. Couldn't grow the evaluation stack");
			return 0;
		}

		break;
		
		case OPCODE_PUSH_TRUE:

		if(!object_push(&state->eval_stack, (nj_object_t*) &state->true_object)){
			
			// #ERROR
			nj_fail(state, "Out of memory. Couldn't grow the evaluation stack");
			return 0;
		}

		break;

		case OPCODE_PUSH_FALSE:

		if(!object_push(&state->eval_stack, (nj_object_t*) &state->false_object)){
			
			// #ERROR
			nj_fail(state, "Out of memory. Couldn't grow the evaluation stack");
			return 0;
		}

		break;
		
		
		case OPCODE_PUSH_INT:
		{

			int64_t value;

			fetch_i64(state, &value);
			
			if(nj_failed(state)) 
				return 0;

			nj_object_t *object = nj_object_from_c_int(state, value);

			if(object == 0) {

				// #ERROR
				nj_fail(state, "Failed to create integer object");
				return 0;
			}

			if(!object_push(&state->eval_stack, object)) {
			
				// #ERROR
				nj_fail(state, "Out of memory. Couldn't grow the evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_PUSH_FLOAT:
		{
			double value;

			fetch_f64(state, &value);

			if(nj_failed(state)) 
				return 0;

			nj_object_t *object = nj_object_from_c_float(state, value);

			if(object == 0) {

				// #ERROR
				nj_fail(state, "Failed to create floating point object");
				return 0;
			}

			if(!object_push(&state->eval_stack, object)) {
			
				// #ERROR
				nj_fail(state, "Out of memory. Couldn't grow the evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_BUILD_ARRAY:
		{

			int64_t count;

			fetch_i64(state, &count);

			if(nj_failed(state))
				return 0;

			if(object_stack_size(&state->eval_stack) < count) {

				// #ERROR
				nj_fail(state, "BUILD_ARRAY with more items than the stack size");
				return 0;
			}

			nj_object_t *object = nj_object_istanciate(state, (nj_object_t*) &state->type_object_array);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				nj_fail(state, "Failed to create array object");
				return 0;
			}

			for(int i = 0; i < count; i++)
				if(!nj_array_insert(state, object, i, object_pop(&state->eval_stack))) {

					// #ERROR
					nj_fail(state, "Failed to insert item into array while building it");
					return 0;
				}

			if(!object_push(&state->eval_stack, object)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}

			if(!object_push(&state->eval_stack, object)) {
			
				// #ERROR
				nj_fail(state, "Out of memory. Couldn't grow the evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_BUILD_DICT:
		{

			int64_t count;

			fetch_i64(state, &count);

			if(nj_failed(state))
				return 0;

			if(object_stack_size(&state->eval_stack) < count * 2) {

				// #ERROR
				nj_fail(state, "BUILD_DICT with more items than the stack size");
				return 0;
			}

			nj_object_t *object = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				nj_fail(state, "Failed to create BUILD_DICT's value");
				return 0;
			}

			for(int i = 0; i < count; i++) {

				nj_object_t *key, *value;

				value = object_pop(&state->eval_stack);
				key   = object_pop(&state->eval_stack);

				if(!nj_object_insert(state, object, key, value)) {

					// #ERROR
					nj_fail(state, "Failed to insert object into dict while building it");
					return 0;
				}
			}

			if(!object_push(&state->eval_stack, object)) {
					
					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow evaluation stack");
					return 0;
			}
			break;
		}

		case OPCODE_PUSH_STRING:
		{

			char *value;

			fetch_string(state, &value);

			if(nj_failed(state)) 
				return 0;

			nj_object_t *object = nj_object_from_c_string_ref(state, value, strlen(value));

			if(object == 0) {

				// #ERROR
				// Failed to create object
				nj_fail(state, "Failed to create string object");
				return 0;
			}

			if(!object_push(&state->eval_stack, object)) {
					
					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow evaluation stack");
					return 0;
			}
			break;
		}
		
		case OPCODE_PUSH_FUNCTION:
		{
		
			uint32_t dest;

			fetch_u32(state, &dest);

			if(nj_failed(state)) 
				return 0;

			uint32_t current_segment = u32_top(&state->segment_stack);

			if(dest >= state->segments[current_segment].code_size) {

				// #ERROR
				// PUSH_FUNCTION refers to an address outside of the code segment
				nj_fail(state, "PUSH_FUNCTION refers to an address outside of the code segment");
				return 0;
			}

			nj_object_t *object = nj_object_from_segment_and_offset(state, current_segment, dest);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				nj_fail(state, "Failed to create PUSH_FUNCTION's value");
				return 0;
			}

			if(!object_push(&state->eval_stack, object)) {
					
					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow evaluation stack");
					return 0;
			}
			break;
		}

		case OPCODE_PUSH_VARIABLE:
		{
			char *variable_name;

			fetch_string(state, &variable_name);

			if(nj_failed(state)) 
				return 0;

			nj_object_t *object = 0;

			if(object_stack_size(&state->vars_stack) > 0)
				object = nj_dictionary_select(state, object_top(&state->vars_stack), variable_name);

			if(object == 0)
				object = nj_dictionary_select(state, state->segments[u32_top(&state->segment_stack)].global_variables_map, variable_name);

			if(object == 0)
				object = nj_dictionary_select(state, state->builtins_map, variable_name);

			if(object == 0) {

				// #ERROR
				// Undefined variable was referenced
				nj_fail(state, "Undefined variable [${zero-terminated-string}] was referenced", variable_name);
				return 0;
			}

			if(!object_push(&state->eval_stack, object)) {
					
					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow evaluation stack");
					return 0;
			}
			break;
		}

		case OPCODE_POP:
		{
			int64_t count;
			
			fetch_i64(state, &count);

			if(nj_failed(state)) 
				return 0;

			if(count < 0) {

				// #ERROR
				// POP's operand is negative
				nj_fail(state, "POP's operand is negative");
				return 0;	
			}

			if(count > object_stack_size(&state->eval_stack)) {

				// #ERROR
				// POP's operand is negative
				nj_fail(state, "POPping more item than there are on the stack");
				return 0;	
			}

			for(int i = 0; i < count; i++)
				object_pop(&state->eval_stack);

			break;
		}

		case OPCODE_ASSIGN:
		{
			char *variable_name;

			fetch_string(state, &variable_name);

			if(nj_failed(state)) 
				return 0;

			if(object_stack_size(&state->eval_stack) == 0) {

				// #ERROR
				// ASSIGN while the stack is empty
				nj_fail(state, "ASSIGN while the stack is empty");
				return 0;
			}

			nj_object_t *dest;

			if(object_stack_size(&state->vars_stack) == 0) {

				dest = state->segments[u32_top(&state->segment_stack)].global_variables_map;

			} else {

				dest = object_top(&state->vars_stack);

			}

			if(!nj_dictionary_insert(state, dest, variable_name, object_top(&state->eval_stack))) {

				// #ERROR
				// Failed to create the variable
				nj_fail(state, "Failed to execute ASSIGN instrucion. Couldn't insert into the variable map");
				return 0;
			}

			break;
		}

		case OPCODE_SELECT_ATTRIBUTE_AND_REPUSH: 
		{
			char *attribute_name;

			fetch_string(state, &attribute_name);

			if(nj_failed(state)) 
				return 0;

			if(object_stack_size(&state->eval_stack) == 0) {

				// #ERROR
				nj_fail(state, "SELECT_ATTRIBUTE_AND_REPUSH on an empty stack");
				return 0;
			}

			nj_object_t *container = object_pop(&state->eval_stack);
			nj_object_t *selected  = nj_object_select_attribute(state, container, attribute_name);

			if(selected == 0) {

				// #ERROR
				nj_fail(state, "Failed to select attribute");
				return 0;
			}

			if(!object_push(&state->eval_stack, selected) || !object_push(&state->eval_stack, container)) {
					
					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow evaluation stack");
					return 0;
			}
			break;
		}

		case OPCODE_SELECT: 
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				nj_fail(state, "SELECT on a stack with less than 2 items");
				return 0;
			}

			nj_object_t *container, *key, *item;

			key 	  = object_pop(&state->eval_stack);
			container = object_top(&state->eval_stack);

			item = nj_object_select(state, container, key);

			if(item == 0) {

				// #ERROR
				nj_fail(state, "Object doesn't contain item");
				return 0;
			}

			*object_top_ref(&state->eval_stack) = item;

			break;
		}

		case OPCODE_INSERT: 
		{
			if(object_stack_size(&state->eval_stack) < 3) {

				// #ERROR
				nj_fail(state, "INSERT on a stack with less than 3 items");
				return 0;
			}

			nj_object_t *container, *key, *item;

			item   	  = object_pop(&state->eval_stack);
			key 	  = object_pop(&state->eval_stack);
			container = object_top(&state->eval_stack);

			if(!nj_object_insert(state, container, key, item)) {

				// #ERROR
				nj_fail(state, "Failed to insert item into object");
				return 0;
			}

			*object_top_ref(&state->eval_stack) = item;
			break;
		}

		case OPCODE_SELECT_ATTRIBUTE: 
		{
			char *attribute_name;

			fetch_string(state, &attribute_name);

			if(nj_failed(state)) 
				return 0;

			if(object_stack_size(&state->eval_stack) == 0) {

				// #ERROR
				nj_fail(state, "SELECT_ATTRIBUTE on an empty stack");
				return 0;
			}

			nj_object_t *container = object_top(&state->eval_stack);
	
			nj_object_t *selected = nj_object_select_attribute(state, container, attribute_name);

			if(selected == 0) {

				// #ERROR
				nj_fail(state, "Failed to select attribute");
				return 0;
			}

			*object_top_ref(&state->eval_stack) = selected;
			break;
		}

		
		case OPCODE_INSERT_ATTRIBUTE: 
		{
			char *attribute_name;

			fetch_string(state, &attribute_name);

			if(nj_failed(state)) 
				return 0;

			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				nj_fail(state, "INSERT_ATTRIBUTE on a stack with less than 2 items");
				return 0;
			}

			nj_object_t *container, *item;

			item      = object_pop(&state->eval_stack);
			container = object_top(&state->eval_stack);

			if(!nj_object_insert_attribute(state, container, attribute_name, item)) {

				// #ERROR
				nj_fail(state, "Failed to insert attribute");
				return 0;
			}

			*object_top_ref(&state->eval_stack) = item;
			break;
		}

		case OPCODE_VARIABLE_MAP_PUSH:
		{
			nj_object_t *dict = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

			if(dict == 0) {

				// #ERROR
				// Failed to create variable map dict
				nj_fail(state, "Failed to create variable map object");
				return 0;
			}

			if(!object_push(&state->vars_stack, dict)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow variable map stack");
				return 0;
			}
			break;
		}

		case OPCODE_VARIABLE_MAP_POP:
		{
			if(object_stack_size(&state->vars_stack) == 0) {

				// #ERROR
				nj_fail(state, "VARIABLE_MAP_POP while the variable map stack is empty");
				return 0;
			}

			object_pop(&state->vars_stack);
			break;
		}

		case OPCODE_CALL: 
		{
			int64_t argc;

			fetch_i64(state, &argc);

			if(nj_failed(state)) 
				return 0;

			if(object_stack_size(&state->eval_stack) < argc + 1) {

				// #ERROR
				// CALL on a stack with not enough items
				nj_fail(state, "CALL on a stack with not enough items");
				return 0;
			}

			nj_object_t *callable = object_nth_from_top(&state->eval_stack, argc + 1);

			if(callable->type == (nj_object_t*) &state->type_object_cfunction) {

				//
				// Handle the call to a C function
				//

				nj_object_t **argv = malloc(sizeof(nj_object_t*) * argc);

				for(int i = 0; i < argc; i++)
					argv[argc-i-1] = object_pop(&state->eval_stack); // Pop the arguments
				object_pop(&state->eval_stack); // Pop the called object

				nj_object_t *result = ((nj_object_cfunction_t*) callable)->routine(state, argc, argv);

				free(argv);

				if(result == 0 && !nj_failed(state)) {

					nj_fail(state, "C function returned NULL but didn't raise an error!");
					return 0;
				}

				if(!object_push(&state->eval_stack, result)) {

					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow evaluation stack");
					return 0;
				}

			} else if(callable->type == (nj_object_t*) &state->type_object_function) {

				//
				// Handle the call to a noja function
				//

				state->argc = argc;

				uint32_t dest_segment, dest_offset;

				dest_segment = ((nj_object_function_t*) callable)->segment;
				dest_offset  = ((nj_object_function_t*) callable)->offset;

				if(!u32_push(&state->segment_stack, dest_segment)) {

					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow segment stack");
					return 0;
				}

				if(!u32_push(&state->offset_stack, dest_offset)) {

					// #ERROR
					nj_fail(state, "Out of memory. Failed to grow segment stack");
					return 0;
				}

			} else {

				nj_fail(state, "CALL on something that is not callable");
				return 0;
			}

			
			break;		
		}

		case OPCODE_EXPECT:
		{
			int64_t argc;

			fetch_i64(state, &argc);

			if(nj_failed(state)) 
				return 0;
		
			if(state->argc < 0) {

				// #ERROR
				nj_fail(state, "EXPECT but the argc wasn't previously set by a CALL instruction");
				return 0;
			}

			if(argc != state->argc) {

				// #ERROR
				nj_fail(state, "Function call didn't provide the required number of arguments");
				return 0;
			}

			state->argc = -1;
			break;
		}

		case OPCODE_RETURN:
		{
			if(u32_stack_size(&state->segment_stack) == 0 || u32_stack_size(&state->offset_stack) == 0) {

				// #ERROR
				nj_fail(state, "RETURN but the call depth is 0");
				return 0;
			}

			u32_pop(&state->segment_stack);
			u32_pop(&state->offset_stack);
			break;
		}

		case OPCODE_JUMP_ABSOLUTE: 
		{
		
			uint32_t dest;

			fetch_u32(state, &dest);

			if(nj_failed(state)) 
				return 0;

			if(dest >= state->segments[u32_top(&state->segment_stack)].code_size) {

				// #ERROR
				nj_fail(state, "JUMP_ABSOLUTE refers to an address outside of the code segment");
				return 0;
			}

			*u32_top_ref(&state->offset_stack) = dest;
			break;
		}
		
		case OPCODE_JUMP_IF_FALSE_AND_POP:
		{
			uint32_t dest;

			fetch_u32(state, &dest);

			if(nj_failed(state)) 
				return 0;

			if(dest >= state->segments[u32_top(&state->segment_stack)].code_size) {

				// #ERROR
				nj_fail(state, "JUMP_IF_FALSE_AND_POP refers to an address outside of the code segment");
				return 0;
			}

			if(object_stack_size(&state->eval_stack) == 0) {

				// #ERROR
				nj_fail(state, "JUMP_IF_FALSE_AND_POP on an empty stack");
				return 0;
			}

			if(!nj_object_test(state, object_pop(&state->eval_stack))) {

				*u32_top_ref(&state->offset_stack) = dest;
			}
		
			break;
		}
		
		case OPCODE_ADD:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				nj_fail(state, "ADD while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_add(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute ADD operation
				nj_fail(state, "Failed to execute ADD");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		#warning "Implement unary operation instructions"
		
		case OPCODE_SUB: 
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// SUB operation on a stack with less than 2 elements
				nj_fail(state, "SUB while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_sub(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute SUB operation
				nj_fail(state, "Failed to execute SUB");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_MUL:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// MUL operation on a stack with less than 2 elements
				nj_fail(state, "MUL while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_mul(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute MUL operation
				nj_fail(state, "Failed to execute MUL");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_DIV:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// DIV operation on a stack with less than 2 elements
				nj_fail(state, "DIV while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_div(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute DIV operation
				nj_fail(state, "Failed to execute DIV");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_MOD:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// MOD operation on a stack with less than 2 elements
				nj_fail(state, "MOD while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_mod(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute MOD operation
				nj_fail(state, "Failed to execute MOD");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_POW:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// POW operation on a stack with less than 2 elements
				nj_fail(state, "POW while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_pow(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute POW operation
				nj_fail(state, "Failed to execute POW");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_NEG: assert(0); break;

		case OPCODE_LSS:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// LSS operation on a stack with less than 2 elements
				nj_fail(state, "LSS while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_lss(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute LSS operation
				nj_fail(state, "Failed to execute LSS");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_GRT: 
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// GRT operation on a stack with less than 2 elements
				nj_fail(state, "GRT while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_grt(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute GRT operation
				nj_fail(state, "Failed to execute GRT");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_LEQ:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// LEQ operation on a stack with less than 2 elements
				nj_fail(state, "LEQ while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_leq(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute LEQ operation
				nj_fail(state, "Failed to execute LEQ");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_GEQ:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// GEQ operation on a stack with less than 2 elements
				nj_fail(state, "GEQ while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_geq(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute GEQ operation
				nj_fail(state, "Failed to execute GEQ");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_EQL:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// EQL operation on a stack with less than 2 elements
				nj_fail(state, "EQL while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_eql(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute EQL operation
				nj_fail(state, "Failed to execute EQL");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_NQL:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// NQL operation on a stack with less than 2 elements
				nj_fail(state, "NQL while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_nql(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute NQL operation
				nj_fail(state, "Failed to execute NQL");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_AND:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// AND operation on a stack with less than 2 elements
				nj_fail(state, "AND while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_and(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute AND operation
				nj_fail(state, "Failed to execute AND");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_OR: 
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// OR operation on a stack with less than 2 elements
				nj_fail(state, "OR while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_or(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute OR operation
				nj_fail(state, "Failed to execute OR");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_NOT: assert(0); break;

		case OPCODE_SHL:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// SHL operation on a stack with less than 2 elements
				nj_fail(state, "SHL while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_shl(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute SHL operation
				nj_fail(state, "Failed to execute SHL");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_SHR:
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// SHR operation on a stack with less than 2 elements
				nj_fail(state, "SHR while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_shr(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute SHR operation
				nj_fail(state, "Failed to execute SHR");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_BITWISE_AND: 
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// BITWISE_AND operation on a stack with less than 2 elements
				nj_fail(state, "BITWISE_AND while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_bitwise_and(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute BITWISE_AND operation
				nj_fail(state, "Failed to execute BITWISE_AND");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_BITWISE_OR:  
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// BITWISE_OR operation on a stack with less than 2 elements
				nj_fail(state, "BITWISE_OR while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_bitwise_or(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute BITWISE_OR operation
				nj_fail(state, "Failed to execute BITWISE_OR");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}

		case OPCODE_BITWISE_XOR: 
		{
			if(object_stack_size(&state->eval_stack) < 2) {

				// #ERROR
				// BITWISE_XOR operation on a stack with less than 2 elements
				nj_fail(state, "BITWISE_XOR while the stack has less than 2 items");
				return 0;
			}

			nj_object_t *left, *right, *result;

			right = object_pop(&state->eval_stack);
			left  = object_pop(&state->eval_stack);
			
			result = nj_object_bitwise_xor(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute BITWISE_XOR operation
				nj_fail(state, "Failed to execute BITWISE_XOR");
				return 0;
			}

			if(!object_push(&state->eval_stack, result)) {

				// #ERROR
				nj_fail(state, "Out of memory. Failed to grow evaluation stack");
				return 0;
			}
			break;
		}


		case OPCODE_BITWISE_NOT: assert(0); break;

		default:
		// #ERROR
		// Unexpected opcode
		nj_fail(state, "Unknown opcode");
		return 0;

	}

	/*
	FILE *fp = fopen("log.txt", "a+");
	
	fprintf(fp, "=== Stack view (%d) ===\n", u32_top(&state->offset_stack));

	object_stack_print(state, &state->eval_stack, fp);

	fprintf(fp, "==================\n");

	fclose(fp);
	*/

	

	return 1;
}

static void fetch_u32(nj_state_t *state, uint32_t *value)
{
	if(u32_top(&state->offset_stack) + sizeof(uint32_t) > state->segments[u32_top(&state->segment_stack)].code_size) {

		nj_fail(state, "Unexpected end of the code segment while fetching an u32");
		return;
	}

	if(value)
		*value = *(uint32_t*) (state->segments[u32_top(&state->segment_stack)].code + u32_top(&state->offset_stack));

	*u32_top_ref(&state->offset_stack) += sizeof(uint32_t);
}

static void fetch_i64(nj_state_t *state, int64_t *value)
{
	if(u32_top(&state->offset_stack) + sizeof(int64_t) > state->segments[u32_top(&state->segment_stack)].code_size) {

		nj_fail(state, "Unexpected end of the code segment while fetching an i64");
		return;
	}

	if(value)
		*value = *(int64_t*) (state->segments[u32_top(&state->segment_stack)].code + u32_top(&state->offset_stack));

	*u32_top_ref(&state->offset_stack) += sizeof(int64_t);
}

static void fetch_f64(nj_state_t *state, double *value)
{
	if(u32_top(&state->offset_stack) + sizeof(double) > state->segments[u32_top(&state->segment_stack)].code_size) {

		nj_fail(state, "Unexpected end of the code segment while fetching an f64");
		return;
	}

	if(value)
		*value = *(double*) (state->segments[u32_top(&state->segment_stack)].code + u32_top(&state->offset_stack));

	*u32_top_ref(&state->offset_stack) += sizeof(double);
}

static void fetch_string(nj_state_t *state, char **value)
{
	if(u32_top(&state->offset_stack) + sizeof(uint32_t) > state->segments[u32_top(&state->segment_stack)].code_size) {

		nj_fail(state, "Unexpected end of the code segment while fetching an u32");
		return;
	}

	uint32_t offset = *(uint32_t*) (state->segments[u32_top(&state->segment_stack)].code + u32_top(&state->offset_stack));

	if(offset >= state->segments[u32_top(&state->segment_stack)].data_size) {

		nj_fail(state, "Fetched data offset points outside of the data segment");
		return;
	}

	if(value)
		*value = state->segments[u32_top(&state->segment_stack)].data + offset;

	*u32_top_ref(&state->offset_stack) += sizeof(uint32_t);
}

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