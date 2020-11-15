
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "noja.h"
#include "utils/basic.h"

int append_segment(nj_state_t *state, char *code, char *data, uint32_t code_size, uint32_t data_size, uint32_t *e_segment)
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
	char *code, *data;
	uint32_t code_size, data_size;

	if(!nj_compile(text, length, &data, &code, &data_size, &code_size, output_builder))
		return 0;

	nj_state_t state;

	if(!nj_state_init(&state, output_builder)) {

		free(code);
		free(data);
		return 0;
	}

	append_segment(&state, code, data, code_size, data_size, 0); // Can't fail 

	u32_push(&state.segment_stack, 0);
	u32_push(&state.offset_stack, 0);

	while(nj_step(&state));

	u32_pop(&state.segment_stack);
	u32_pop(&state.offset_stack);

	int result = !state.failed;

	nj_state_deinit(&state);
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