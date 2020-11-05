
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "noja.h"

int inspect(state_t *state)
{
	int _continue = 0;
	int _quitting = 0;

	while(!_quitting && !_continue) {


		fprintf(stdout, ">> ");



		// Get a zero-terminated string from the cmdline

		char *buffer;
		int   length;

		if(!read_line(stdin, &buffer, &length)) {

			fprintf(stderr, "Out of memory!\n");
			return -1;
		}

		// Split it into tokens

		char **argv;
		int    argc;

		if(!tokenize_buffer(buffer, &argv, &argc)) {

			free(buffer);
			fprintf(stderr, "Out of memory!\n");
			return -1;
		}

		if(argc == 0) {

			_continue = 1;

		} else if(!strcmp(argv[0], "vars")) {

			for(size_t i = 0; i < state->variable_maps_count; i++) {

				object_print(state, state->variable_maps[state->variable_maps_count-i-1], stdout);
				fprintf(stdout, "\n");
			}

		} else if(!strcmp(argv[0], "pc")) {

			fprintf(stdout, "The current program counter is %d\n", state->program_counters[state->call_depth-1]);

		} else if(!strcmp(argv[0], "continue")) {

			_continue = 1;	
		
		} else if(!strcmp(argv[0], "quit")) {

			_quitting = 1;
		
		} else if(!strcmp(argv[0], "stack")) {

			if(state->stack_item_count == 0) {

				fprintf(stdout, "The stack is empty\n");
	
			} else {

				for(uint32_t i = 0; i < state->stack_item_count; i++) {

					fprintf(stdout, "%-3d: ", i);
					object_print(state, state->stack[i], stdout);
					fprintf(stdout, "\n");
				}

			}

		} else {

			fprintf(stdout, "Unknown instruction [%s]\n", argv[0]);
		}

		// Release resources

		free(argv);
		free(buffer);
	}

	return !_quitting;
}

int inspect_run(executable_t *executable, char *error_buffer, int error_buffer_size)
{
	state_t state;
	if(!state_init(&state, executable))
		return 0;

	int result;

	if(inspect(&state)) {

		while((result = step(&state, error_buffer, error_buffer_size)) > 0)

			if(!inspect(&state))
				break;
	}

	state_deinit(&state);
	return result >= 0;
}

int run(executable_t *executable, char *error_buffer, int error_buffer_size)
{
	state_t state;
	if(!state_init(&state, executable))
		return 0;

	int result;

	while((result = step(&state, error_buffer, error_buffer_size)) > 0);

	state_deinit(&state);
	return result >= 0;
}

int main()
{
	char buffer[4096];

	executable_t *executable = compile_from_file("samples/sample.noja", buffer, sizeof(buffer));

	if(!executable) {

		fprintf(stderr, ">> Failed to generate executable: %s\n", buffer);
		return -1;
	}

	if(!run(executable, buffer, sizeof(buffer))) {

		fprintf(stderr, "Runtime error: %s\n", buffer);

	} else {


		fprintf(stderr, "All good.\n");
	}

	free(executable);

	return 0;
}