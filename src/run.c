
#include <stdlib.h>
#include <assert.h>
#include "noja.h"
#include "bytecode.h"

int gc_requires_collection(state_t *state)
{
	return state->overflow_allocations != 0;
}

int gc_collect(state_t *state)
{
	return 1;
}

int state_init(state_t *state, executable_t *executable)
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

	state->executable = executable;

	state->heap_size = 4096;
	state->heap_used = 0;
	state->overflow_allocations = 0;

	state->stack_item_count = 0;
	state->stack_item_count_max = 128;

	state->continue_destinations_depth = 0;
	state->break_destinations_depth = 0;
	state->program_counters_depth = 1;
	state->program_counters[0] = 0;

	state->variable_maps[0] = object_istanciate(state, (object_t*) &dict_type_object);
	state->variable_maps_count = 1;
	state->variable_maps_count_max = 128;

	assert(state->variable_maps[0]);

	return 1;
}

void state_deinit(state_t *state)
{
	free(state->heap);
	free(state->stack);
	free(state->variable_maps);
}

static int stack_is_full(state_t *state)
{
	return state->stack_item_count == state->stack_item_count_max;
}

static int fetch_u32(state_t *state, uint32_t *value)
{
	if(state->program_counters[state->program_counters_depth-1] + sizeof(uint32_t) > state->executable->code_length)
		return 0;

	if(value)
		*value = *(uint32_t*) (state->executable->code + state->program_counters[state->program_counters_depth-1]);

	state->program_counters[state->program_counters_depth-1] += sizeof(uint32_t);

	return 1;
}

static int fetch_i64(state_t *state, int64_t *value)
{
	if(state->program_counters[state->program_counters_depth-1] + sizeof(int64_t) > state->executable->code_length)
		return 0;

	if(value)
		*value = *(int64_t*) (state->executable->code + state->program_counters[state->program_counters_depth-1]);

	state->program_counters[state->program_counters_depth-1] += sizeof(int64_t);

	return 1;
}

static int fetch_f64(state_t *state, double *value)
{
	if(state->program_counters[state->program_counters_depth-1] + sizeof(double) > state->executable->code_length)
		return 0;

	if(value)
		*value = *(double*) (state->executable->code + state->program_counters[state->program_counters_depth-1]);

	state->program_counters[state->program_counters_depth-1] += sizeof(double);

	return 1;
}

static int fetch_string(state_t *state, char **value)
{
	if(state->program_counters[state->program_counters_depth-1] + sizeof(uint32_t) > state->executable->code_length)
		
		return 0;

	uint32_t offset = *(uint32_t*) (state->executable->code + state->program_counters[state->program_counters_depth-1]);

	if(offset >= state->executable->data_length)

		return 0;

	if(value)
		*value = state->executable->data + offset;

	state->program_counters[state->program_counters_depth-1] += sizeof(uint32_t);

	return 1;
}

int step(state_t *state, char *error_buffer, int error_buffer_size)
{
	(void) error_buffer;
	(void) error_buffer_size;

	uint32_t opcode;

	if(!fetch_u32(state, &opcode))

		// Unexpected end of code
		return -1;

	switch(opcode) {

		case OPCODE_NOPE:
		// Do nothing
		break;

		case OPCODE_QUIT:
		state->program_counters[state->program_counters_depth-1] -= sizeof(uint32_t);
		return 0;
			
		case OPCODE_PUSH_NULL:

		if(stack_is_full(state)) {

			// #ERROR
			// The stack is full!
			report(error_buffer, error_buffer_size, "PUSH_NULL while out of stack");
			return -1;
		}

		state->stack[state->stack_item_count++] = (object_t*) &object_null;
		break;
		
		case OPCODE_PUSH_TRUE:

		if(stack_is_full(state)) {

			// #ERROR
			// The stack is full!
			report(error_buffer, error_buffer_size, "PUSH_TRUE while out of stack");
			return -1;
		}

		state->stack[state->stack_item_count++] = (object_t*) &object_true;
		break;

		case OPCODE_PUSH_FALSE:
		
		if(stack_is_full(state)) {

			// #ERROR
			// The stack is full!
			report(error_buffer, error_buffer_size, "PUSH_FALSE while out of stack");
			return -1;
		}

		state->stack[state->stack_item_count++] = (object_t*) &object_false;
		break;
		
		
		case OPCODE_PUSH_INT:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_INT on a full stack
				report(error_buffer, error_buffer_size, "PUSH_INT while out of stack");
				return -1;
			}

			int64_t value;

			if(!fetch_i64(state, &value)) {

				// #ERROR
				// Unexpected end of code
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching PUSH_INT's operand");
				return -1;
			}
				
			object_t *object = object_from_cint(state, value);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				report(error_buffer, error_buffer_size, "Failed to create PUSH_INT's integer value");
				return -1;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_FLOAT:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_FLOAT on a full stack
				report(error_buffer, error_buffer_size, "PUSH_FLOAT while out of stack");
				return -1;
			}

			double value;

			if(!fetch_f64(state, &value)) {

				// #ERROR
				// Unexpected end of code
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching PUSH_FLOAT's operand");
				return -1;
			}

			object_t *object = object_from_cfloat(state, value);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				report(error_buffer, error_buffer_size, "Failed to create PUSH_FLOAT's value");
				return -1;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_ARRAY:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_ARRAY on a full stack
				report(error_buffer, error_buffer_size, "PUSH_ARRAY while out of stack");
				return -1;
			}

			object_t *object = object_istanciate(state, &array_type_object);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				report(error_buffer, error_buffer_size, "Failed to create PUSH_ARRAY's value");
				return -1;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_DICT:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_DICT on a full stack
				report(error_buffer, error_buffer_size, "PUSH_DICT while out of stack");
				return -1;
			}

			object_t *object = object_istanciate(state, &dict_type_object);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				report(error_buffer, error_buffer_size, "Failed to create PUSH_DICT's value");
				return -1;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_STRING:
		fetch_string(state, 0);
		assert(0);
		#warning "Implement OPCODE_PUSH_STRING"
		break;
		
		case OPCODE_PUSH_FUNCTION:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_FUNCTION on a full stack
				report(error_buffer, error_buffer_size, "PUSH_FUNCTION while out of stack");
				return -1;
			}
		
			uint32_t dest;

			if(!fetch_u32(state, &dest)) {

				// #ERROR
				// Unexpected end of code while fetching PUSH_FUNCTION's operand
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching PUSH_FUNCTION's operand");
				return -1;
			}

			if(dest >= state->executable->code_length) {

				// #ERROR
				// PUSH_FUNCTION refers to an address outside of the code segment
				report(error_buffer, error_buffer_size, "PUSH_FUNCTION refers to an address outside of the code segment");
				return -1;
			}

			object_t *object = object_from_executable_and_offset(state, state->executable, dest);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				report(error_buffer, error_buffer_size, "Failed to create PUSH_FUNCTION's value");
				return -1;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_VARIABLE:
		{
			char *variable_name;

			if(!fetch_string(state, &variable_name)) {

				// #ERROR
				// Unexpected error of code or offset pointrs outside of the data segment
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching PUSH_VARIABLE's operand or it's operand points outside of the data segment");
				return -1;
			}

			object_t *object = dict_cselect(state, state->variable_maps[state->variable_maps_count-1], variable_name);

			if(object == 0) {

				// #ERROR
				// Undefined variable was referenced
				report(error_buffer, error_buffer_size, "PUSH_VARIABLE references an undefined variable");
				return -1;
			}

			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// Out of stack
				report(error_buffer, error_buffer_size, "PUSH_VARIABLE while out of stack");
				return -1;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_POP:
		{
			int64_t count;
			
			if(!fetch_i64(state, &count)) {

				// #ERROR
				// Unexpected end of code while fetching POP's operand
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching POP's operand");
				return -1;
			}

			if(count < 0) {

				// #ERROR
				// POP's operand is negative
				report(error_buffer, error_buffer_size, "POP's operand is negative");
				return -1;	
			}

			if(count > state->stack_item_count) {

				// #ERROR
				// POP's operand is negative
				report(error_buffer, error_buffer_size, "POPping more item than there are on the stack");
				return -1;	
			}

			state->stack_item_count -= count;

			break;
		}

		case OPCODE_ASSIGN:
		{
			char *variable_name;

			if(!fetch_string(state, &variable_name)) {

				// #ERROR
				// Unexpected error of code or offset pointrs outside of the data segment
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching PUSH_VARIABLE's operand or it's operand points outside of the data segment");
				return -1;
			}

			if(state->variable_maps_count == 0) {

				// #ERROR
				// ASSIGN while the variable map stack is empty
				report(error_buffer, error_buffer_size, "ASSIGN while the variable maps stack is empty");
				return -1;
			}

			if(state->stack_item_count == 0) {

				// #ERROR
				// ASSIGN while the stack is empty
				report(error_buffer, error_buffer_size, "ASSIGN while the stack is empty");
				return -1;
			}

			if(!dict_cinsert(state, state->variable_maps[state->variable_maps_count-1], variable_name, state->stack[state->stack_item_count-1])) {

				// #ERROR
				// Failed to create the variable
				report(error_buffer, error_buffer_size, "Failed to execute ASSIGN instrucion. Couldn't insert into the variable map");
				return -1;
			}

			break;
		}

		case OPCODE_SELECT: 
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				report(error_buffer, error_buffer_size, "SELECT on a stack with less than 2 items");
				return -1;
			}

			object_t *container, *key, *item;

			container = state->stack[state->stack_item_count-2];
			key 	  = state->stack[state->stack_item_count-1];

			item = object_select(state, container, key);

			if(item == 0) {

				// #ERROR
				report(error_buffer, error_buffer_size, "Object doesn't contain item");
				return -1;
			}

			state->stack[state->stack_item_count-1] = item;
			break;
		}

		case OPCODE_INSERT: 
		{
			if(state->stack_item_count < 3) {

				// #ERROR
				report(error_buffer, error_buffer_size, "INSERT on a stack with less than 3 items");
				return -1;
			}

			object_t *container, *key, *item;

			container = state->stack[state->stack_item_count-3];
			key 	  = state->stack[state->stack_item_count-2];
			item   	  = state->stack[state->stack_item_count-1];

			if(!object_insert(state, container, key, item)) {

				// #ERROR
				report(error_buffer, error_buffer_size, "Failed to insert item into object");
				return -1;
			}

			break;
		}

		case OPCODE_SELECT_ATTRIBUTE: 
		fetch_string(state, 0); 
		assert(0); 
		#warning "Implement OPCODE_SELECT_ATTRIBUTE"
		break;
		
		case OPCODE_INSERT_ATTRIBUTE: 
		fetch_string(state, 0); 
		assert(0); 
		#warning "Implement OPCODE_INSERT_ATTRIBUTE"
		break;

		case OPCODE_VARIABLE_MAP_PUSH:
		{
			if(state->variable_maps_count == state->variable_maps_count_max) {

				// #ERROR
				// The variable set stack is full!
				report(error_buffer, error_buffer_size, "VARIABLE_MAP_PUSH while out of variable map stack");
				return -1;
			}

			object_t *dict = object_istanciate(state, (object_t*) &dict_type_object);

			if(dict == 0) {

				// #ERROR
				// Failed to create variable map dict
				report(error_buffer, error_buffer_size, "Failed to create variable map object");
				return -1;
			}

			state->variable_maps[state->variable_maps_count++] = dict;

			break;
		}

		case OPCODE_VARIABLE_MAP_POP:
		{
			if(state->variable_maps_count == 0) {

				// #ERROR
				// Popping variable map while the stack is empty
				report(error_buffer, error_buffer_size, "VARIABLE_MAP_POP while the variable map stack is empty");
				return -1;
			}

			state->variable_maps_count--;
			break;
		}

		case OPCODE_BREAK: 
		{
			if(state->break_destinations_depth == 0) {

				// #ERROR
				report(error_buffer, error_buffer_size, "BREAK but no break destination was set");
				return -1;
			}
			
			state->program_counters[state->program_counters_depth-1] = state->break_destinations[state->break_destinations_depth-1];
			break;
		}
		
		case OPCODE_BREAK_DESTINATION_PUSH: 
		{
			uint32_t dest;

			if(!fetch_u32(state, &dest)) {

				// #ERROR
				// Unexpected end of code while fetching OPCODE_BREAK_DESTINATION_PUSH's operand
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching OPCODE_BREAK_DESTINATION_PUSH's operand");
				return -1;
			}

			if(dest >= state->executable->code_length) {

				// #ERROR
				// OPCODE_BREAK_DESTINATION_PUSH refers to an address outside of the code segment
				report(error_buffer, error_buffer_size, "OPCODE_BREAK_DESTINATION_PUSH refers to an address outside of the code segment");
				return -1;
			}

			if(state->continue_destinations_depth == 16) {

				// #ERROR
				// break destination stack is full
				report(error_buffer, error_buffer_size, "OPCODE_BREAK_DESTINATION_PUSH but the break destination stack is full");
				return -1;
			}

			state->break_destinations[state->break_destinations_depth++] = dest;
			break;
		}
		
		case OPCODE_BREAK_DESTINATION_POP: 
		{
			if(state->break_destinations_depth == 0) {

				// #ERROR
				report(error_buffer, error_buffer_size, "OPCODE_BREAK_DESTINATION_POP but the break destination stack is empty");
				return -1;
			}

			state->break_destinations_depth--;
			break;
		}

		case OPCODE_CONTINUE: 
		{
			if(state->continue_destinations_depth == 0) {

				// #ERROR
				report(error_buffer, error_buffer_size, "CONTINUE but no break destination was set");
				return -1;
			}
			
			state->program_counters[state->program_counters_depth-1] = state->continue_destinations[state->continue_destinations_depth-1];
			break;
		}
		
		case OPCODE_CONTINUE_DESTINATION_PUSH: 
		{
			uint32_t dest;

			if(!fetch_u32(state, &dest)) {

				// #ERROR
				// Unexpected end of code while fetching OPCODE_CONTINUE_DESTINATION_PUSH's operand
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching OPCODE_CONTINUE_DESTINATION_PUSH's operand");
				return -1;
			}

			if(dest >= state->executable->code_length) {

				// #ERROR
				// OPCODE_CONTINUE_DESTINATION_PUSH refers to an address outside of the code segment
				report(error_buffer, error_buffer_size, "OPCODE_CONTINUE_DESTINATION_PUSH refers to an address outside of the code segment");
				return -1;
			}

			if(state->continue_destinations_depth == 16) {

				// #ERROR
				// continue destination stack is full
				report(error_buffer, error_buffer_size, "OPCODE_CONTINUE_DESTINATION_PUSH but the continue destination stack is full");
				return -1;
			}

			state->continue_destinations[state->continue_destinations_depth++] = dest;
			break;
		}
		
		case OPCODE_CONTINUE_DESTINATION_POP:
		{
			if(state->continue_destinations_depth == 0) {

				report(error_buffer, error_buffer_size, "OPCODE_CONTINUE_DESTINATION_POP but the continue destination stack is empty");
				return -1;
			}

			state->continue_destinations_depth--;
			break;
		}

		case OPCODE_CALL: 
		fetch_u32(state, 0); 
		assert(0);
		#warning "Implement OPCODE_CALL"
		break;

		case OPCODE_RETURN:
		assert(0);
		#warning "Implement OPCODE_RETURN"
		break;

		case OPCODE_JUMP_ABSOLUTE: 
		{
		
			uint32_t dest;

			if(!fetch_u32(state, &dest)) {

				// #ERROR
				// Unexpected end of code while fetching JUMP_ABSOLUTE's operand
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching JUMP_ABSOLUTE's operand");
				return -1;
			}

			if(dest >= state->executable->code_length) {

				// #ERROR
				// JUMP_ABSOLUTE refers to an address outside of the code segment
				report(error_buffer, error_buffer_size, "JUMP_ABSOLUTE refers to an address outside of the code segment");
				return -1;
			}

			state->program_counters[state->program_counters_depth-1] = dest;
			break;
		}
		
		case OPCODE_JUMP_IF_FALSE_AND_POP:
		{
			uint32_t dest;

			if(!fetch_u32(state, &dest)) {

				// #ERROR
				// Unexpected end of code while fetching JUMP_IF_FALSE_AND_POP's operand
				report(error_buffer, error_buffer_size, "Unexpected end of code while fetching JUMP_IF_FALSE_AND_POP's operand");
				return -1;
			}

			if(dest >= state->executable->code_length) {

				// #ERROR
				// JUMP_IF_FALSE_AND_POP refers to an address outside of the code segment
				report(error_buffer, error_buffer_size, "JUMP_IF_FALSE_AND_POP refers to an address outside of the code segment");
				return -1;
			}

			if(state->stack_item_count == 0) {

				// #ERROR
				// JUMP_IF_FALSE_AND_POP on an empty stack
				report(error_buffer, error_buffer_size, "JUMP_IF_FALSE_AND_POP on an empty stack");
				return -1;
			}

			if(!object_test(state, state->stack[--state->stack_item_count]))
				state->program_counters[state->program_counters_depth-1] = dest;
		
			break;
		}

		case OPCODE_PRINT:
		{
			if(state->stack_item_count == 0) {

				report(error_buffer, error_buffer_size, "OPCODE_PRINT while the stack is empty");
				return -1;
			}

			object_print(state, state->stack[state->stack_item_count-1], stdout);
			break;
		}
		
		case OPCODE_ADD:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// ADD operation on a stack with less than 2 elements
				report(error_buffer, error_buffer_size, "ADD while the stack has less than 2 items");
				return -1;
			}

			object_t *left, *right, *result;

			left  = state->stack[--state->stack_item_count];
			right = state->stack[--state->stack_item_count];
			result = object_add(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute ADD operation
				report(error_buffer, error_buffer_size, "Failed to execute ADD");
				return -1;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		#warning "Implement operation instructions other than ADD"
		case OPCODE_SUB: assert(0); break;
		case OPCODE_MUL: assert(0); break;
		case OPCODE_DIV: assert(0); break;
		case OPCODE_MOD: assert(0); break;
		case OPCODE_POW: assert(0); break;
		case OPCODE_NEG: assert(0); break;
		case OPCODE_LSS: assert(0); break;
		case OPCODE_GRT: assert(0); break;
		case OPCODE_LEQ: assert(0); break;
		case OPCODE_GEQ: assert(0); break;
		case OPCODE_EQL: assert(0); break;
		case OPCODE_NQL: assert(0); break;
		case OPCODE_AND: assert(0); break;
		case OPCODE_OR: assert(0); break;
		case OPCODE_NOT: assert(0); break;
		case OPCODE_SHL: assert(0); break;
		case OPCODE_SHR: assert(0); break;
		case OPCODE_BITWISE_AND: assert(0); break;
		case OPCODE_BITWISE_OR: assert(0); break;
		case OPCODE_BITWISE_XOR: assert(0); break;
		case OPCODE_BITWISE_NOT: assert(0); break;

		default:
		// #ERROR
		// Unexpected opcode
		report(error_buffer, error_buffer_size, "Unknown opcode");
		return -1;

	}

	if(gc_requires_collection(state))
		if(!gc_collect(state)) {

			// #ERROR Out of memory
			report(error_buffer, error_buffer_size, "Out of memory");
			return -1;
		}

	return 1;
}