
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "utils/basic.h"
#include "noja.h"

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
	char *code, *data;
	uint32_t code_size, data_size;

	char *text;
	int length;

	if(!load_text(path, &text, &length)) {

		nj_fail(state, "Failed to open \"${zero-terminated-string}\"", path);
		return 0;
	}

	if(!nj_compile(text, length, &data, &code, &data_size, &code_size, state->output_builder)) {

		nj_fail(state, "Failed to generate bytecode for \"${zero-terminated-string}\"", path);
		
		free(text);
		return 0;
	}

	free(text);

	//
	// Create a new segment
	//

	uint32_t imported_segment;

	if(!append_segment(state, code, data, code_size, data_size, &imported_segment)) {

		// #ERROR

		free(code);
		free(data);

		nj_fail(state, "Out of memory. Failed to grow segment array");
		return 0;
	}

	//
	// Run the segment
	//

	u32_push(&state->segment_stack, imported_segment);
	u32_push(&state->offset_stack, 0);

	while(nj_step(state));

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

int nj_import_as(nj_state_t *state, const char *name)
{

	if(object_stack_size(&state->eval_stack) == 0) {

		// #ERROR
		nj_fail(state, "OPCODE_IMPORT_AS on an empty stack");
		return 0;
	}

	// Pop an object from the evaluation stack
	// and try to get the path from it.
	
	nj_object_t *path_object = object_pop(&state->eval_stack);

	if(path_object->type != (nj_object_t*) &state->type_object_string) {

		// #ERROR
		nj_fail(state, "The imported path expression is not a string");
		return 0;
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
		return 0;

	if(!nj_dictionary_insert(state, current_map(state), name, map)) {

		// #ERROR
		nj_fail(state, "Failed to create imported module variable. Couldn't insert into the variable map");
		return 0;
	}

	return 1;
}

int nj_import(nj_state_t *state)
{

	if(object_stack_size(&state->eval_stack) == 0) {

		// #ERROR
		nj_fail(state, "OPCODE_IMPORT on an empty stack");
		return 0;
	}

	// Pop an object from the evaluation stack
	// and try to get the path from it.
	
	nj_object_t *path_object = object_pop(&state->eval_stack);

	if(path_object->type != (nj_object_t*) &state->type_object_string) {

		// #ERROR
		nj_fail(state, "The imported path expression is not a string");
		return 0;
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
		return 0;

	//
	// Insert the symbols in it
	//

	if(!nj_dictionary_merge_in(state, current_map(state), map)) {

		// #ERROR
		nj_fail(state, "Failed to create import module symnols");
		return 0;
	}

	return 1;
}