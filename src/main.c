
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "noja.h"

int load_text(const char *path, char **e_content, int *e_length)
{
	FILE *fp = fopen(path, "rb");

	if(fp == 0)
		return 0;

	size_t length;

	fseek(fp, 0, SEEK_END);

	length = ftell(fp);
	
	if(e_length)
		*e_length = length;

	fseek(fp, 0, SEEK_SET);

	*e_content = malloc(length + 1);

	if(*e_content == 0) {

		fclose(fp);
		return 0;
	}

	if(fread(*e_content, 1, length, fp) != length) {

		free(*e_content);
		
		*e_content = 0;
		
		if(e_length)
			*e_length = 0;

		fclose(fp);
		return 0;
	}

	(*e_content)[length] = '\0';

	fclose(fp);
	return 1;
}

void print_code(executable_t *executable)
{
	char *code, *data;
	int code_length, data_length;

	code = executable->code;
	data = executable->data;
	code_length = executable->code_length;
	data_length = executable->data_length;

	{
		int i = 0;

		while(i < data_length) {

			char c = data[i];

			if(i == 0 || data[i-1] == '\0')
				fprintf(stderr, "%-4d | \"", i);

			switch(c) {
				case '\0': fprintf(stderr, "\"\n"); break;
				case '\n': fprintf(stderr, "\\n"); break;
				case '\t': fprintf(stderr, "\\t"); break;
				case '\r': fprintf(stderr, "\\s"); break;
				default: fprintf(stderr, "%c", c); break;
			}

			i++;
		}
	}

	int i = 0;

	while(i < code_length) {

		uint32_t opcode = *(uint32_t*) (code + i);

		const char *name = get_opcode_name(opcode);

		fprintf(stderr, "%-4d | (%-3d) %s ", i, opcode, name);

		i += sizeof(uint32_t);

		const char *operands = get_instruction_operands(opcode);

		assert(operands);

		int j = 0;
		while(operands[j]) {

			switch(operands[j]) {
				case 'i':
				{
					fprintf(stderr, "%ld", *(int64_t*) (code + i));

					i += sizeof(int64_t);
					break;
				}
				case 's':
				{
					fprintf(stderr, "\"%s\"", data + *(uint32_t*) (code + i));
					//fprintf(stderr, "%d", *(uint32_t*) (code + i));

					i += sizeof(uint32_t);
					break;
				}
				case 'f':
				{
					fprintf(stderr, "%f", *(double*) (code + i));

					i += sizeof(double);
					break;
				}

				case 'a':
				{
					fprintf(stderr, "%d", *(uint32_t*) (code + i));

					i += sizeof(uint32_t);
					break;
				}

				default: fprintf(stderr, "Unexpected operand type [%c]\n", operands[j]);
			}

			if(operands[j+1])
				fprintf(stderr, ", ");

			j++;
		}

		fprintf(stderr, "\n");
	}
}

executable_t *compile(char *source, int length, char *error_buffer, int error_buffer_size)
{

	token_array_t tokens;
	pool_t *pool;
	node_t *node;

	if(!tokenize(source, length, &tokens))

		// Failed to tokenize
		return 0;

	if(!parse(&tokens, source, &pool, &node))

		// Failed to parse
		return 0;


	if(!check(node, source, error_buffer, error_buffer_size))

		// There was a semantic error!
		return 0;


	executable_t *executable = generate(node);

	pool_destroy(pool);;
	token_array_deinit(&tokens);

	return executable;
}

executable_t *compile_from_file(char *path, char *error_buffer, int error_buffer_size)
{
	char *source;
	int length;

	if(!load_text(path, &source, &length))

		// Failed to load file contents
		return 0;

	executable_t *executable = compile(source, length, error_buffer, error_buffer_size);

	free(source);

	return executable;
}

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

		} else if(!strcmp(argv[0], "pc")) {

			fprintf(stdout, "The current program counter is %d\n", state->program_counters[state->program_counters_depth-1]);

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
	state_init(&state, executable);

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
	state_init(&state, executable);

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

	fprintf(stderr, ">> Generated executable\n");

	print_code(executable);

	if(!run(executable, buffer, sizeof(buffer))) {

		fprintf(stderr, "Runtime error: %s\n", buffer);

	} else {


		fprintf(stderr, "All good.\n");
	}

	free(executable);

	return 0;
}