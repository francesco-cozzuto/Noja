
#include <string.h>
#include <assert.h>
#include "noja.h"

static void fetch_u32(state_t *state, uint32_t *value);
static void fetch_i64(state_t *state, int64_t *value);
static void fetch_f64(state_t *state, double *value);
static void fetch_string(state_t *state, char **value);
static void fail(state_t *state, const char *fmt, ...);
static int  failed(state_t *state);

int step(state_t *state)
{
	uint32_t opcode;

	fetch_u32(state, &opcode);

	if(failed(state)) return 0;

	switch(opcode) {

		case OPCODE_NOPE:
		// Do nothing
		break;

		case OPCODE_QUIT:
		state->program_counters[state->call_depth-1] -= sizeof(uint32_t);
		return 0;
			
		case OPCODE_PUSH_NULL:

		if(state->stack_item_count == state->stack_item_count_max) {

			// #ERROR
			// The stack is full!

			fail(state, "PUSH_NULL while out of stack");
			return 0;
		}

		state->stack[state->stack_item_count++] = (object_t*) &object_null;
		break;
		
		case OPCODE_PUSH_TRUE:

		if(state->stack_item_count == state->stack_item_count_max) {

			// #ERROR
			// The stack is full!
			fail(state, "PUSH_TRUE while out of stack");
			return 0;
		}

		state->stack[state->stack_item_count++] = (object_t*) &object_true;
		break;

		case OPCODE_PUSH_FALSE:
		
		if(state->stack_item_count == state->stack_item_count_max) {

			// #ERROR
			// The stack is full!
			fail(state, "PUSH_FALSE while out of stack");
			return 0;
		}

		state->stack[state->stack_item_count++] = (object_t*) &object_false;
		break;
		
		
		case OPCODE_PUSH_INT:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_INT on a full stack
				fail(state, "PUSH_INT while out of stack");
				return 0;
			}

			int64_t value;

			fetch_i64(state, &value);
			
			if(failed(state)) 
				return 0;

			object_t *object = object_from_cint(state, value);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				fail(state, "Failed to create PUSH_INT's integer value");
				return 0;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_FLOAT:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_FLOAT on a full stack
				fail(state, "PUSH_FLOAT while out of stack");
				return 0;
			}

			double value;

			fetch_f64(state, &value);

			if(failed(state)) 
				return 0;

			object_t *object = object_from_cfloat(state, value);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				fail(state, "Failed to create PUSH_FLOAT's value");
				return 0;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_ARRAY:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_ARRAY on a full stack
				fail(state, "PUSH_ARRAY while out of stack");
				return 0;
			}

			object_t *object = object_istanciate(state, (object_t*) &array_type_object);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				fail(state, "Failed to create PUSH_ARRAY's value");
				return 0;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_DICT:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_DICT on a full stack
				fail(state, "PUSH_DICT while out of stack");
				return 0;
			}

			object_t *object = object_istanciate(state, (object_t*) &dict_type_object);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				fail(state, "Failed to create PUSH_DICT's value");
				return 0;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_STRING:
		{

			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_STRING on a full stack
				fail(state, "PUSH_STRING while out of stack");
				return 0;
			}

			char *value;

			fetch_string(state, &value);

			if(failed(state)) 
				return 0;

			object_t *object = object_from_cstring_ref(state, value, strlen(value));

			if(object == 0) {

				// #ERROR
				// Failed to create object
				fail(state, "Failed to create PUSH_STRING's value");
				return 0;
			}

			state->stack[state->stack_item_count++] = object;

			break;
		}
		
		case OPCODE_PUSH_FUNCTION:
		{
			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// PUSH_FUNCTION on a full stack
				fail(state, "PUSH_FUNCTION while out of stack");
				return 0;
			}
		
			uint32_t dest;

			fetch_u32(state, &dest);

			if(failed(state)) 
				return 0;

			if(dest >= state->executable_stack[state->call_depth-1]->code_length) {

				// #ERROR
				// PUSH_FUNCTION refers to an address outside of the code segment
				fail(state, "PUSH_FUNCTION refers to an address outside of the code segment");
				return 0;
			}

			object_t *object = object_from_executable_and_offset(state, state->executable_stack[state->call_depth-1], dest);

			if(object == 0) {

				// #ERROR
				// Failed to create object
				fail(state, "Failed to create PUSH_FUNCTION's value");
				return 0;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_PUSH_VARIABLE:
		{
			char *variable_name;

			fetch_string(state, &variable_name);

			if(failed(state)) 
				return 0;

			object_t *object = dict_cselect(state, state->variable_maps[state->variable_maps_count-1], variable_name);

			if(object == 0 && state->variable_maps_count > 1)
				object = dict_cselect(state, state->variable_maps[0], variable_name);

			if(object == 0)
				object = dict_cselect(state, state->builtins_map, variable_name);

			if(object == 0) {

				// #ERROR
				// Undefined variable was referenced
				fail(state, "Undefined variable [${zero-terminated-string}] was referenced", variable_name);
				return 0;
			}

			if(state->stack_item_count == state->stack_item_count_max) {

				// #ERROR
				// Out of stack
				fail(state, "PUSH_VARIABLE while out of stack");
				return 0;
			}

			state->stack[state->stack_item_count++] = object;
			break;
		}

		case OPCODE_POP:
		{
			int64_t count;
			
			fetch_i64(state, &count);

			if(failed(state)) 
				return 0;

			if(count < 0) {

				// #ERROR
				// POP's operand is negative
				fail(state, "POP's operand is negative");
				return 0;	
			}

			if(count > state->stack_item_count) {

				// #ERROR
				// POP's operand is negative
				fail(state, "POPping more item than there are on the stack");
				return 0;	
			}

			state->stack_item_count -= count;

			break;
		}

		case OPCODE_ASSIGN:
		{
			char *variable_name;

			fetch_string(state, &variable_name);

			if(failed(state)) 
				return 0;

			if(state->variable_maps_count == 0) {

				// #ERROR
				// ASSIGN while the variable map stack is empty
				fail(state, "ASSIGN while the variable maps stack is empty");
				return 0;
			}

			if(state->stack_item_count == 0) {

				// #ERROR
				// ASSIGN while the stack is empty
				fail(state, "ASSIGN while the stack is empty");
				return 0;
			}

			if(!dict_cinsert(state, state->variable_maps[state->variable_maps_count-1], variable_name, state->stack[state->stack_item_count-1])) {

				// #ERROR
				// Failed to create the variable
				fail(state, "Failed to execute ASSIGN instrucion. Couldn't insert into the variable map");
				return 0;
			}

			break;
		}

		case OPCODE_SELECT: 
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				fail(state, "SELECT on a stack with less than 2 items");
				return 0;
			}

			object_t *container, *key, *item;

			key 	  = state->stack[--state->stack_item_count];
			container = state->stack[--state->stack_item_count];

			item = object_select(state, container, key);

			if(item == 0) {

				// #ERROR
				fail(state, "Object doesn't contain item");
				return 0;
			}

			state->stack[state->stack_item_count++] = item;
			break;
		}

		case OPCODE_INSERT: 
		{
			if(state->stack_item_count < 3) {

				// #ERROR
				fail(state, "INSERT on a stack with less than 3 items");
				return 0;
			}

			object_t *container, *key, *item;

			container = state->stack[state->stack_item_count-3];
			key 	  = state->stack[state->stack_item_count-2];
			item   	  = state->stack[state->stack_item_count-1];

			if(!object_insert(state, container, key, item)) {

				// #ERROR
				fail(state, "Failed to insert item into object");
				return 0;
			}

			break;
		}

		case OPCODE_SELECT_ATTRIBUTE: 
		{
			char *attribute_name;

			fetch_string(state, &attribute_name);

			if(failed(state)) 
				return 0;

			if(state->stack_item_count == 0) {

				// #ERROR
				fail(state, "SELECT_ATTRIBUTE on an empty stack");
				return 0;
			}

			object_t *selected = object_select_attribute(state, state->stack[state->stack_item_count-1], attribute_name);

			if(selected == 0) {

				// #ERROR
				fail(state, "Failed to select attribute");
				return 0;
			}

			state->stack[state->stack_item_count-1] = selected;
			break;
		}

		
		case OPCODE_INSERT_ATTRIBUTE: 
		{
			char *attribute_name;

			fetch_string(state, &attribute_name);

			if(failed(state)) 
				return 0;

			if(state->stack_item_count < 2) {

				// #ERROR
				fail(state, "INSERT_ATTRIBUTE on a stack with less than 2 items");
				return 0;
			}

			object_t *container, *item;

			item      = state->stack[--state->stack_item_count];
			container = state->stack[--state->stack_item_count];

			if(!object_insert_attribute(state, container, attribute_name, item)) {

				// #ERROR
				fail(state, "Failed to insert attribute");
				return 0;
			}

			state->stack[state->stack_item_count++] = object_select_attribute(state, container, attribute_name);
			break;
		}

		case OPCODE_VARIABLE_MAP_PUSH:
		{
			if(state->variable_maps_count == state->variable_maps_count_max) {

				// #ERROR
				// The variable set stack is full!
				fail(state, "VARIABLE_MAP_PUSH while out of variable map stack");
				return 0;
			}

			object_t *dict = object_istanciate(state, (object_t*) &dict_type_object);

			if(dict == 0) {

				// #ERROR
				// Failed to create variable map dict
				fail(state, "Failed to create variable map object");
				return 0;
			}

			state->variable_maps[state->variable_maps_count++] = dict;

			break;
		}

		case OPCODE_VARIABLE_MAP_POP:
		{
			if(state->variable_maps_count == 0) {

				// #ERROR
				// Popping variable map while the stack is empty
				fail(state, "VARIABLE_MAP_POP while the variable map stack is empty");
				return 0;
			}

			state->variable_maps_count--;
			break;
		}

		case OPCODE_BREAK: 
		{
			if(state->break_destinations_depth == 0) {

				// #ERROR
				fail(state, "BREAK but no break destination was set");
				return 0;
			}
			
			state->program_counters[state->call_depth-1] = state->break_destinations[state->break_destinations_depth-1];
			break;
		}
		
		case OPCODE_BREAK_DESTINATION_PUSH: 
		{
			uint32_t dest;

			fetch_u32(state, &dest);

			if(failed(state)) 
				return 0;

			if(dest >= state->executable_stack[state->call_depth-1]->code_length) {

				// #ERROR
				// OPCODE_BREAK_DESTINATION_PUSH refers to an address outside of the code segment
				fail(state, "OPCODE_BREAK_DESTINATION_PUSH refers to an address outside of the code segment");
				return 0;
			}

			if(state->continue_destinations_depth == 16) {

				// #ERROR
				// break destination stack is full
				fail(state, "OPCODE_BREAK_DESTINATION_PUSH but the break destination stack is full");
				return 0;
			}

			state->break_destinations[state->break_destinations_depth++] = dest;
			break;
		}
		
		case OPCODE_BREAK_DESTINATION_POP: 
		{
			if(state->break_destinations_depth == 0) {

				// #ERROR
				fail(state, "OPCODE_BREAK_DESTINATION_POP but the break destination stack is empty");
				return 0;
			}

			state->break_destinations_depth--;
			break;
		}

		case OPCODE_CONTINUE: 
		{
			if(state->continue_destinations_depth == 0) {

				// #ERROR
				fail(state, "CONTINUE but no break destination was set");
				return 0;
			}
			
			state->program_counters[state->call_depth-1] = state->continue_destinations[state->continue_destinations_depth-1];
			break;
		}
		
		case OPCODE_CONTINUE_DESTINATION_PUSH: 
		{
			uint32_t dest;

			fetch_u32(state, &dest);

			if(failed(state)) 
				return 0;

			if(dest >= state->executable_stack[state->call_depth-1]->code_length) {

				// #ERROR
				// OPCODE_CONTINUE_DESTINATION_PUSH refers to an address outside of the code segment
				fail(state, "OPCODE_CONTINUE_DESTINATION_PUSH refers to an address outside of the code segment");
				return 0;
			}

			if(state->continue_destinations_depth == 16) {

				// #ERROR
				// continue destination stack is full
				fail(state, "OPCODE_CONTINUE_DESTINATION_PUSH but the continue destination stack is full");
				return 0;
			}

			state->continue_destinations[state->continue_destinations_depth++] = dest;
			break;
		}
		
		case OPCODE_CONTINUE_DESTINATION_POP:
		{
			if(state->continue_destinations_depth == 0) {

				fail(state, "OPCODE_CONTINUE_DESTINATION_POP but the continue destination stack is empty");
				return 0;
			}

			state->continue_destinations_depth--;
			break;
		}

		case OPCODE_CALL: 
		{

			if(state->call_depth == 16) {

				fail(state, "CALL but the maximum call depth was reached");
				return 0;
			}

			int64_t argc;

			fetch_i64(state, &argc);

			if(failed(state)) 
				return 0;

			if(state->stack_item_count < argc + 1) {

				// #ERROR
				// CALL on a stack with not enough items
				fail(state, "CALL on a stack with not enough items");
				return 0;
			}
				
			object_t *callable = state->stack[state->stack_item_count - argc - 1];

			if(callable->type == (object_t*) &cfunction_type_object) {

				object_t *result = ((object_cfunction_t*) callable)->routine(state, argc, state->stack + state->stack_item_count - argc);

				if(result == 0) {

					fail(state, "C function returned NULL");
					return 0;
				}

				state->stack_item_count -= argc + 1;
				state->stack[state->stack_item_count++] = result;

			} else if(callable->type == (object_t*) &function_type_object) {

				state->argc = argc;

				state->executable_stack[state->call_depth] = ((object_function_t*) callable)->executable;
				state->program_counters[state->call_depth] = ((object_function_t*) callable)->offset;
				state->call_depth++;

			} else {

				fail(state, "CALL on something that is not callable");
				return 0;
			}

			
			break;		
		}

		case OPCODE_EXPECT:
		{
			int64_t argc;

			fetch_i64(state, &argc);

			if(failed(state)) 
				return 0;
		
			if(state->argc < 0) {

				// #ERROR
				fail(state, "EXPECT but the argc wasn't previously set by a CALL instruction");
				return 0;
			}

			if(argc != state->argc) {

				// #ERROR
				fail(state, "Function call didn't provide the required number of arguments");
				return 0;
			}

			state->argc = -1;
			break;
		}

		case OPCODE_RETURN:
		{
			if(state->call_depth == 0) {
				fail(state, "RETURN but the call depth is 0");
				return 0;
			}

			state->call_depth--;
			break;
		}

		case OPCODE_JUMP_ABSOLUTE: 
		{
		
			uint32_t dest;

			fetch_u32(state, &dest);

			if(failed(state)) 
				return 0;

			if(dest >= state->executable_stack[state->call_depth-1]->code_length) {

				// #ERROR
				// JUMP_ABSOLUTE refers to an address outside of the code segment
				fail(state, "JUMP_ABSOLUTE refers to an address outside of the code segment");
				return 0;
			}

			state->program_counters[state->call_depth-1] = dest;
			break;
		}
		
		case OPCODE_JUMP_IF_FALSE_AND_POP:
		{
			uint32_t dest;

			fetch_u32(state, &dest);

			if(failed(state)) 
				return 0;

			if(dest >= state->executable_stack[state->call_depth-1]->code_length) {

				// #ERROR
				// JUMP_IF_FALSE_AND_POP refers to an address outside of the code segment
				fail(state, "JUMP_IF_FALSE_AND_POP refers to an address outside of the code segment");
				return 0;
			}

			if(state->stack_item_count == 0) {

				// #ERROR
				// JUMP_IF_FALSE_AND_POP on an empty stack
				fail(state, "JUMP_IF_FALSE_AND_POP on an empty stack");
				return 0;
			}

			if(!object_test(state, state->stack[--state->stack_item_count]))
				state->program_counters[state->call_depth-1] = dest;
		
			break;
		}
		
		case OPCODE_ADD:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// ADD operation on a stack with less than 2 elements
				fail(state, "ADD while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_add(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute ADD operation
				fail(state, "Failed to execute ADD");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		#warning "Implement unary operation instructions"
		
		case OPCODE_SUB: 
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// SUB operation on a stack with less than 2 elements
				fail(state, "SUB while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_sub(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute SUB operation
				fail(state, "Failed to execute SUB");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_MUL:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// MUL operation on a stack with less than 2 elements
				fail(state, "MUL while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_mul(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute MUL operation
				fail(state, "Failed to execute MUL");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_DIV:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// DIV operation on a stack with less than 2 elements
				fail(state, "DIV while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_div(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute DIV operation
				fail(state, "Failed to execute DIV");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_MOD:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// MOD operation on a stack with less than 2 elements
				fail(state, "MOD while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_mod(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute MOD operation
				fail(state, "Failed to execute MOD");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_POW:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// POW operation on a stack with less than 2 elements
				fail(state, "POW while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_pow(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute POW operation
				fail(state, "Failed to execute POW");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_NEG: assert(0); break;

		case OPCODE_LSS:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// LSS operation on a stack with less than 2 elements
				fail(state, "LSS while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_lss(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute LSS operation
				fail(state, "Failed to execute LSS");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_GRT: 
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// GRT operation on a stack with less than 2 elements
				fail(state, "GRT while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_grt(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute GRT operation
				fail(state, "Failed to execute GRT");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_LEQ:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// LEQ operation on a stack with less than 2 elements
				fail(state, "LEQ while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_leq(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute LEQ operation
				fail(state, "Failed to execute LEQ");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_GEQ:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// GEQ operation on a stack with less than 2 elements
				fail(state, "GEQ while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_geq(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute GEQ operation
				fail(state, "Failed to execute GEQ");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_EQL:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// EQL operation on a stack with less than 2 elements
				fail(state, "EQL while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_eql(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute EQL operation
				fail(state, "Failed to execute EQL");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_NQL:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// NQL operation on a stack with less than 2 elements
				fail(state, "NQL while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_nql(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute NQL operation
				fail(state, "Failed to execute NQL");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_AND:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// AND operation on a stack with less than 2 elements
				fail(state, "AND while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_and(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute AND operation
				fail(state, "Failed to execute AND");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_OR: 
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// OR operation on a stack with less than 2 elements
				fail(state, "OR while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_or(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute OR operation
				fail(state, "Failed to execute OR");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_NOT: assert(0); break;

		case OPCODE_SHL:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// SHL operation on a stack with less than 2 elements
				fail(state, "SHL while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_shl(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute SHL operation
				fail(state, "Failed to execute SHL");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_SHR:
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// SHR operation on a stack with less than 2 elements
				fail(state, "SHR while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_shr(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute SHR operation
				fail(state, "Failed to execute SHR");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_BITWISE_AND: 
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// BITWISE_AND operation on a stack with less than 2 elements
				fail(state, "BITWISE_AND while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_bitwise_and(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute BITWISE_AND operation
				fail(state, "Failed to execute BITWISE_AND");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_BITWISE_OR:  
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// BITWISE_OR operation on a stack with less than 2 elements
				fail(state, "BITWISE_OR while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_bitwise_or(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute BITWISE_OR operation
				fail(state, "Failed to execute BITWISE_OR");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}

		case OPCODE_BITWISE_XOR: 
		{
			if(state->stack_item_count < 2) {

				// #ERROR
				// BITWISE_XOR operation on a stack with less than 2 elements
				fail(state, "BITWISE_XOR while the stack has less than 2 items");
				return 0;
			}

			object_t *left, *right, *result;

			right = state->stack[--state->stack_item_count];
			left  = state->stack[--state->stack_item_count];
			
			result = object_bitwise_xor(state, left, right);

			if(result == 0) {

				// #ERROR
				// Failed to execute BITWISE_XOR operation
				fail(state, "Failed to execute BITWISE_XOR");
				return 0;
			}

			state->stack[state->stack_item_count++] = result;
			break;
		}


		case OPCODE_BITWISE_NOT: assert(0); break;

		default:
		// #ERROR
		// Unexpected opcode
		fail(state, "Unknown opcode");
		return 0;

	}

	/*
	printf("=== Stack view (%d) ===\n", state->program_counters[state->call_depth-1]);

	for(size_t i = 0; i < state->stack_item_count; i++) {

		printf("%ld: [", i);
		object_print(state, state->stack[i], stdout);
		printf("]\n");

	}

	printf("==================\n");
	
	getc(stdin);
	*/

	return 1;
}

static void fetch_u32(state_t *state, uint32_t *value)
{
	if(state->program_counters[state->call_depth-1] + sizeof(uint32_t) > state->executable_stack[state->call_depth-1]->code_length) {

		fail(state, "Unexpected end of the code segment while fetching an u32");
		return;
	}

	if(value)
		*value = *(uint32_t*) (state->executable_stack[state->call_depth-1]->code + state->program_counters[state->call_depth-1]);

	state->program_counters[state->call_depth-1] += sizeof(uint32_t);
}

static void fetch_i64(state_t *state, int64_t *value)
{
	if(state->program_counters[state->call_depth-1] + sizeof(int64_t) > state->executable_stack[state->call_depth-1]->code_length) {

		fail(state, "Unexpected end of the code segment while fetching an i64");
		return;
	}

	if(value)
		*value = *(int64_t*) (state->executable_stack[state->call_depth-1]->code + state->program_counters[state->call_depth-1]);

	state->program_counters[state->call_depth-1] += sizeof(int64_t);
}

static void fetch_f64(state_t *state, double *value)
{
	if(state->program_counters[state->call_depth-1] + sizeof(double) > state->executable_stack[state->call_depth-1]->code_length) {

		fail(state, "Unexpected end of the code segment while fetching an f64");
		return;
	}

	if(value)
		*value = *(double*) (state->executable_stack[state->call_depth-1]->code + state->program_counters[state->call_depth-1]);

	state->program_counters[state->call_depth-1] += sizeof(double);
}

static void fetch_string(state_t *state, char **value)
{
	if(state->program_counters[state->call_depth-1] + sizeof(uint32_t) > state->executable_stack[state->call_depth-1]->code_length) {

		fail(state, "Unexpected end of the code segment while fetching an u32");
		return;
	}

	uint32_t offset = *(uint32_t*) (state->executable_stack[state->call_depth-1]->code + state->program_counters[state->call_depth-1]);

	if(offset >= state->executable_stack[state->call_depth-1]->data_length) {

		fail(state, "Fetched data offset points outside of the data segment");
		return;
	}

	if(value)
		*value = state->executable_stack[state->call_depth-1]->data + offset;

	state->program_counters[state->call_depth-1] += sizeof(uint32_t);
}

static void fail(state_t *state, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	string_builder_append_p(state->output_builder, fmt, args);
	va_end(args);

	state->failed = 1;
}

static int failed(state_t *state)
{
	return state->failed;
}