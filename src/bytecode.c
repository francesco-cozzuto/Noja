
#include "bytecode.h"

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

	[OPCODE_PRINT] = "",

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

const char *get_instruction_operands(int opcode)
{
	return operand_types[opcode];
}

const char *get_opcode_name(int opcode)
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

		case OPCODE_PRINT: return "PRINT";

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