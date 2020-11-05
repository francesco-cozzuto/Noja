
#include <assert.h>
#include "builtins.h"

static const char *operand_types[] = {
	
	[OPCODE_NOPE] = "",
	[OPCODE_QUIT] = "",

	[OPCODE_PUSH_NULL] = "",
	[OPCODE_PUSH_TRUE] = "",
	[OPCODE_PUSH_FALSE] = "",
	[OPCODE_PUSH_INT] = "i",
	[OPCODE_PUSH_FLOAT] = "f",
	[OPCODE_PUSH_STRING] = "s",
	[OPCODE_PUSH_ARRAY] = "",
	[OPCODE_PUSH_DICT] = "",
	[OPCODE_PUSH_FUNCTION] = "a",
	[OPCODE_PUSH_VARIABLE] = "s",

	[OPCODE_POP] = "i",

	[OPCODE_ASSIGN] = "s",
	[OPCODE_SELECT] = "",
	[OPCODE_INSERT] = "",
	[OPCODE_SELECT_ATTRIBUTE] = "s",
	[OPCODE_INSERT_ATTRIBUTE] = "s",

	[OPCODE_VARIABLE_MAP_PUSH] = "",
	[OPCODE_VARIABLE_MAP_POP] = "",

	[OPCODE_BREAK] = "",
	[OPCODE_BREAK_DESTINATION_PUSH] = "a",
	[OPCODE_BREAK_DESTINATION_POP] = "",

	[OPCODE_CONTINUE] = "",
	[OPCODE_CONTINUE_DESTINATION_PUSH] = "a",
	[OPCODE_CONTINUE_DESTINATION_POP] = "",

	[OPCODE_CALL] = "i",
	[OPCODE_EXPECT] = "i",
	[OPCODE_RETURN] = "",

	[OPCODE_JUMP_ABSOLUTE] = "a",
	[OPCODE_JUMP_IF_FALSE_AND_POP] = "a",

	[OPCODE_ADD] = "",
	[OPCODE_SUB] = "",
	[OPCODE_MUL] = "",
	[OPCODE_DIV] = "",
	[OPCODE_MOD] = "",
	[OPCODE_POW] = "",
	[OPCODE_NEG] = "",
	[OPCODE_LSS] = "",
	[OPCODE_GRT] = "",
	[OPCODE_LEQ] = "",
	[OPCODE_GEQ] = "",
	[OPCODE_EQL] = "",
	[OPCODE_NQL] = "",
	[OPCODE_AND] = "",
	[OPCODE_OR] = "",
	[OPCODE_NOT] = "",
	[OPCODE_SHL] = "",
	[OPCODE_SHR] = "",
	[OPCODE_BITWISE_AND] = "",
	[OPCODE_BITWISE_OR] = "",
	[OPCODE_BITWISE_XOR] = "",
	[OPCODE_BITWISE_NOT] = "",
};

static const char *get_instruction_operands(int opcode)
{
	return operand_types[opcode];
}

static const char *get_opcode_name(int opcode)
{
	switch(opcode) {

		case OPCODE_NOPE: return "NOPE";
		case OPCODE_QUIT: return "QUIT";
			
		case OPCODE_PUSH_NULL: return "PUSH_NULL";
		case OPCODE_PUSH_TRUE: return "PUSH_TRUE";
		case OPCODE_PUSH_FALSE: return "PUSH_FALSE";
		case OPCODE_PUSH_INT: return "PUSH_INT";
		case OPCODE_PUSH_FLOAT: return "PUSH_FLOAT";
		case OPCODE_PUSH_STRING: return "PUSH_STRING";
		case OPCODE_PUSH_ARRAY: return "PUSH_ARRAY";
		case OPCODE_PUSH_DICT: return "PUSH_DICT";
		case OPCODE_PUSH_FUNCTION: return "PUSH_FUNCTION";
		case OPCODE_PUSH_VARIABLE: return "PUSH_VARIABLE";

		case OPCODE_POP: return "POP";

		case OPCODE_ASSIGN: return "ASSIGN";
		case OPCODE_SELECT: return "SELECT";
		case OPCODE_INSERT: return "INSERT";
		case OPCODE_SELECT_ATTRIBUTE: return "SELECT_ATTRIBUTE";
		case OPCODE_INSERT_ATTRIBUTE: return "INSERT_ATTRIBUTE";

		case OPCODE_VARIABLE_MAP_PUSH: return "VARIABLE_MAP_PUSH";
		case OPCODE_VARIABLE_MAP_POP: return "VARIABLE_MAP_POP";

		case OPCODE_BREAK: return "BREAK";
		case OPCODE_BREAK_DESTINATION_PUSH: return "BREAK_DESTINATION_PUSH";
		case OPCODE_BREAK_DESTINATION_POP: return "BREAK_DESTINATION_POP";

		case OPCODE_CONTINUE: return "CONTINUE";
		case OPCODE_CONTINUE_DESTINATION_PUSH: return "CONTINUE_DESTINATION_PUSH";
		case OPCODE_CONTINUE_DESTINATION_POP: return "CONTINUE_DESTINATION_POP";

		case OPCODE_CALL: return "CALL";
		case OPCODE_EXPECT: return "EXPECT";
		case OPCODE_RETURN: return "RETURN";

		case OPCODE_JUMP_ABSOLUTE: return "JUMP_ABSOLUTE";
		case OPCODE_JUMP_IF_FALSE_AND_POP: return "JUMP_IF_FALSE_AND_POP";

		case OPCODE_ADD: return "ADD";
		case OPCODE_SUB: return "SUB";
		case OPCODE_MUL: return "MUL";
		case OPCODE_DIV: return "DIV";
		case OPCODE_MOD: return "MOD";
		case OPCODE_POW: return "POW";
		case OPCODE_NEG: return "NEG";
		case OPCODE_LSS: return "LSS";
		case OPCODE_GRT: return "GRT";
		case OPCODE_LEQ: return "LEQ";
		case OPCODE_GEQ: return "GEQ";
		case OPCODE_EQL: return "EQL";
		case OPCODE_NQL: return "NQL";
		case OPCODE_AND: return "AND";
		case OPCODE_OR: return "OR";
		case OPCODE_NOT: return "NOT";
		case OPCODE_SHL: return "SHL";
		case OPCODE_SHR: return "SHR";
		case OPCODE_BITWISE_AND: return "BITWISE_AND";
		case OPCODE_BITWISE_OR: return "BITWISE_OR";
		case OPCODE_BITWISE_XOR: return "BITWISE_XOR";
		case OPCODE_BITWISE_NOT: return "BITWISE_NOT";
	}

	return "???";
}


object_t *builtin_disassemble(state_t *state, int argc, object_t **argv)
{
	(void) state;

	if(argc != 0)

		// #ERROR
		// Unexpected arguments 
		return 0;

	char *code, *data;
	int code_length, data_length;

	code = state->executable_stack[state->call_depth-1]->code;
	data = state->executable_stack[state->call_depth-1]->data;
	code_length = state->executable_stack[state->call_depth-1]->code_length;
	data_length = state->executable_stack[state->call_depth-1]->data_length;

	{
		int i = 0;

		while(i < data_length) {

			char c = data[i];

			if(i == 0 || data[i-1] == '\0')
				fprintf(stdout, "%-4d | \"", i);

			switch(c) {
				case '\0': fprintf(stdout, "\"\n"); break;
				case '\n': fprintf(stdout, "\\n"); break;
				case '\t': fprintf(stdout, "\\t"); break;
				case '\r': fprintf(stdout, "\\s"); break;
				default: fprintf(stdout, "%c", c); break;
			}

			i++;
		}
	}

	int i = 0;

	while(i < code_length) {

		uint32_t opcode = *(uint32_t*) (code + i);

		const char *name = get_opcode_name(opcode);

		fprintf(stdout, "%-4d | %s ", i, name);

		i += sizeof(uint32_t);

		const char *operands = get_instruction_operands(opcode);

		assert(operands);

		int j = 0;
		while(operands[j]) {

			switch(operands[j]) {
				case 'i':
				{
					fprintf(stdout, "%ld", *(int64_t*) (code + i));

					i += sizeof(int64_t);
					break;
				}
				case 's':
				{
					uint32_t offset = *(uint32_t*) (code + i);

					fprintf(stdout, "%d (\"%s\")", offset, data + offset);

					i += sizeof(uint32_t);
					break;
				}
				case 'f':
				{
					fprintf(stdout, "%f", *(double*) (code + i));

					i += sizeof(double);
					break;
				}

				case 'a':
				{
					fprintf(stdout, "%d", *(uint32_t*) (code + i));

					i += sizeof(uint32_t);
					break;
				}

				default: fprintf(stdout, "Unexpected operand type [%c]\n", operands[j]);
			}

			if(operands[j+1])
				fprintf(stdout, ", ");

			j++;
		}

		fprintf(stdout, "\n");
	}

	return (object_t*) &object_null;
}