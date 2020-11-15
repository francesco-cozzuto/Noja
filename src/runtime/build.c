
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <string.h>
#include "noja.h"

#define BYTES_PER_CODE_CHUNK 1024
#define BYTES_PER_DATA_CHUNK 1024

typedef struct block_t block_t;
typedef struct program_builder_t program_builder_t;

typedef struct data_chunk_t data_chunk_t;
struct data_chunk_t {
	data_chunk_t *next;
	uint32_t used;
	char body[BYTES_PER_DATA_CHUNK];
};

typedef struct chunk_t chunk_t;

struct chunk_t {
	chunk_t *next;
	uint32_t used;
	char body[BYTES_PER_CODE_CHUNK];
};

typedef struct gap_t gap_t;
struct gap_t {

	gap_t *prev;

	chunk_t *chunk;  		  //
	uint32_t offset_in_chunk; // The pointer to the gap
};

typedef struct label_t label_t;
struct label_t {
	
	label_t *prev;

	block_t *block;  		  // 
	uint32_t offset_in_block; // The pointer value to fill the gaps with

	gap_t *tail_gap;

};

typedef struct node_location_t node_location_t;
struct node_location_t {
	
	node_location_t *prev;

	node_t *node;
	block_t *block;
	uint32_t offset_in_block;
};

struct block_t {

	program_builder_t *builder;
	block_t *next;

	uint32_t offset;
	uint32_t length;

	chunk_t head, *tail;

};

struct program_builder_t {

	node_location_t *tail_node_location;

	label_t *tail_label;

	block_t *head_block,
			*tail_block;

	data_chunk_t *head_data_chunk,
				 *tail_data_chunk;
	uint32_t data_length;

	jmp_buf env;

};

enum {
	END = 0,
	U8 , S8 ,
	U16, S16,
	U32, S32,
	U64, S64, 
	F32,
	F64,
	STR,
	LBL,
};

block_t *block_create(program_builder_t *builder);
int 	 block_append(block_t *block, ...);
label_t *label_create(block_t *block);
void 	 label_points_here(block_t *block, label_t *label);

static void throw(program_builder_t *builder)
{
	longjmp(builder->env, 1);
}

void node_code_starts_here(node_t *node, block_t *block)
{
	node_location_t *node_location = malloc(sizeof(node_location_t));

	if(node_location == 0)
		throw(block->builder);

	node_location->node = node;
	node_location->block = block;
	node_location->offset_in_block = block->length;

	node_location->prev = block->builder->tail_node_location;
	block->builder->tail_node_location = node_location; 
}

label_t *label_create(block_t *block)
{
	label_t *label = malloc(sizeof(label_t));

	assert(label);

	label->block = 0;
	label->offset_in_block = 0;
	label->tail_gap = 0;

	// Add to the label list of the builder

	label->prev = block->builder->tail_label;
	block->builder->tail_label = label;

	return label;
}

void release_resources_on_abort(program_builder_t *builder)
{

}

int generate(ast_t ast, char **e_data, char **e_code, uint32_t *e_data_size, uint32_t *e_code_size)
{
	int failed;

	program_t program = build_program(ast, &failed);

	if(failed)
		return 0;

	*e_data = program.data;
	*e_code = program.code;
	*e_data_size = program.data_length;
	*e_code_size = program.code_length;
	return 1;
}

static void node_compile(block_t *block, label_t *break_destination, label_t *continue_destination, node_t *node);

program_t build_program(ast_t ast, int *failed)
{

	//
	// Setup stuff
	//

	program_builder_t builder;
	memset(&builder, 0, sizeof(program_builder_t));

	{
		builder.head_data_chunk = malloc(sizeof(data_chunk_t));

		if(builder.head_data_chunk == 0) {

			assert(0);

			if(failed) *failed = 1;
			return (program_t) { 0 };
		}

		builder.head_data_chunk->used = 0;
		builder.head_data_chunk->next = NULL;

		builder.tail_data_chunk = builder.head_data_chunk;
	}

	if(setjmp(builder.env)) {

		assert(0);

		release_resources_on_abort(&builder);

		if(failed) *failed = 1;
		return (program_t) { 0 };
	}

	//
	// Do the thing
	//

	{
		block_t *first_block = block_create(&builder);

		node_compile(first_block, 0, 0, ast.root);
		block_append(first_block, U32, OPCODE_QUIT, END);
	}

	//
	// Serialize the code
	//

	uint32_t length = 0; // The lengths of the code. To be calculated!

	// Assigns offsets to the blocks and
	// calculate the size of the whole
	// sode segment.
	{
		block_t *block = builder.head_block;

		while(block) {

			block->offset = length;
			length += block->length;

			block = block->next;
		}
	}

	//
	// Set node locations
	//

	{
		node_location_t *location = builder.tail_node_location;

		while(location) {

			// Set the location of the node inside of the node

			location->node->in_code = location->block->offset + location->offset_in_block;

			// Free node location structure

			{
				node_location_t *prev_location = location->prev;
				free(location);
				location = prev_location;
			}
		}	
	}

	// Resolve the gaps associated to each label and while 
	// doing it free all of the label and gap structures

	{
		label_t *label = builder.tail_label;

		while(label) {

			// Resolve the gaps associated to this label

			gap_t *gap = label->tail_gap;

			while(gap) {


				// Fill the gap

				*(uint32_t*) (gap->chunk->body + gap->offset_in_chunk) = (uint32_t) (label->block->offset + label->offset_in_block);

				// Free the gap and go to the next one

				{
					gap_t *prev_gap = gap->prev;
					free(gap);
					gap = prev_gap;
				}
			}

			// Free the label and go to the next one

			{
				label_t *prev_label = label->prev;
				free(label);
				label = prev_label;
			}
		}
	}

	char *code = malloc(length);

	assert(code);

	// Write to the allocated space

	{
		uint32_t written = 0;

		block_t *block = builder.head_block;

		while(block) {

			chunk_t *chunk = &block->head;

			{
				memcpy(code + written, chunk->body, chunk->used);

				written += chunk->used;
			}

			chunk = chunk->next;

			while(chunk) {

				// Write the content

				memcpy(code + written, chunk->body, chunk->used);

				written += chunk->used;

				// Free the chunk and go to the next one

				{
					chunk_t *next_chunk = chunk->next;
					free(chunk);
					chunk = next_chunk;
				}
			}

			// Free the block and go to the next one

			{
				block_t *next_block = block->next;
				free(block);
				block = next_block;
			}
		}
	}

	//
	// Serialize the data
	//

	char *data = malloc(builder.data_length);

	assert(data);

	{
		uint32_t written = 0;

		data_chunk_t *chunk = builder.head_data_chunk;

		while(chunk) {

			memcpy(data + written, chunk->body, chunk->used);

			written += chunk->used;

			// Free the chunk and go to the next one

			{
				data_chunk_t *next_chunk = chunk->next;
				free(chunk);
				chunk = next_chunk;
			}
		}
	}

	// Done!

	if(failed) *failed = 0;
	return (program_t) { .code = code, .data = data, .code_length = length, .data_length = builder.data_length };
}

void label_points_here(block_t *block, label_t *label)
{
	label->block = block;
	label->offset_in_block = block->length;
}

block_t *block_create(program_builder_t *builder)
{
	block_t *block = malloc(sizeof(block_t));

	assert(block);

	block->builder = builder;
	block->next = 0;

	block->offset = 0;
	block->length = 0;

	block->head.next = NULL;
	block->head.used = 0;

	block->tail = &block->head;

	// Append block to builder

	if(!builder->head_block) {

		builder->head_block = block;

	} else {

		builder->tail_block->next = block;
	}

	builder->tail_block = block;

	return block;
}

block_t *sub_block_create(block_t *parent_block)
{
	return block_create(parent_block->builder);
}

static int block_ensure_space(block_t *block, uint32_t required_space)
{
	if(BYTES_PER_CODE_CHUNK - block->tail->used < required_space) {

		// resize
		chunk_t *chunk = malloc(sizeof(chunk_t));

		if(chunk == 0)
			return 0;

		chunk->next = 0;
		chunk->used = 0;
		block->tail->next = chunk;
		block->tail = chunk;
	}

	return 1;
}

#define DEF_APPENDER(suffix, T) 							\
static int block_append_ ## suffix (block_t *block, T v) 	\
{															\
	if(!block_ensure_space(block, sizeof(T))) 					\
		return 0;											\
															\
	*(T*) (block->tail->body + block->tail->used) = v;		\
															\
	block->tail->used += sizeof(T);							\
	block->length += sizeof(T);								\
	return 1;												\
}

DEF_APPENDER(u8 , uint8_t);
DEF_APPENDER(u16, uint16_t);
DEF_APPENDER(u32, uint32_t);
DEF_APPENDER(u64, uint64_t);
DEF_APPENDER(s8 , int8_t);
DEF_APPENDER(s16, int16_t);
DEF_APPENDER(s32, int32_t);
DEF_APPENDER(s64, int64_t);
DEF_APPENDER(f32, float);
DEF_APPENDER(f64, double);

#undef DEF_APPENDER

static int block_append_string(block_t *block, const char *string)
{
	if(!block_append_u32(block, block->builder->data_length))
		return 0;

	do { 

		if(block->builder->tail_data_chunk->used == BYTES_PER_DATA_CHUNK) {

			data_chunk_t *chunk = malloc(sizeof(data_chunk_t));

			if(chunk == 0)
				return 0;

			chunk->used = 0;
			chunk->next = NULL;

			block->builder->tail_data_chunk->next = chunk;
			block->builder->tail_data_chunk = chunk;
		}

		block->builder->tail_data_chunk->body[block->builder->tail_data_chunk->used++] = *string;
		block->builder->data_length++;
		
		if(*string == '\0')
			break;
		
		string++;

	} while(1);

	return 1;
}

int block_append(block_t *block, ...)
{
	va_list args;
	va_start(args, block);

	int done = 0;

	while(!done) {

		int type = va_arg(args, int);

		switch(type) {

			case U8 : if(!block_append_u8 (block, va_arg(args, uint8_t)))  return 0; break;
			case U16: if(!block_append_u16(block, va_arg(args, uint16_t))) return 0; break;
			case U32: if(!block_append_u32(block, va_arg(args, uint32_t))) return 0; break;
			case U64: if(!block_append_u64(block, va_arg(args, uint64_t))) return 0; break;

			case S8 : if(!block_append_s8 (block, va_arg(args, int8_t)))  return 0; break;
			case S16: if(!block_append_s16(block, va_arg(args, int16_t))) return 0; break;
			case S32: if(!block_append_s32(block, va_arg(args, int32_t))) return 0; break;
			case S64: if(!block_append_s64(block, va_arg(args, int64_t))) return 0; break;

			case F32: if(!block_append_f32(block, va_arg(args, float)))  return 0; break;
			case F64: if(!block_append_f64(block, va_arg(args, double))) return 0; break;

			case STR: if(!block_append_string(block, va_arg(args, char*))) return 0; break;

			case LBL:
			{
				chunk_t *chunk;
				uint32_t offset_in_chunk;

				if(!block_ensure_space(block, sizeof(uint32_t))) // This is to make sure that the
					return 0;									 // current chunk is the one the
																 // next u32 will be written to.
																 
				chunk = block->tail;				 // These two represent the location where
				offset_in_chunk = block->tail->used; // the value is missing.

				label_t *label = va_arg(args, label_t*);

				gap_t *gap = malloc(sizeof(gap_t));

				if(gap == NULL)
					return 0;

				if(!block_append_u32(block, 0)) {

					free(gap);
					return 0;
				}

				gap->chunk = chunk;
				gap->offset_in_chunk = offset_in_chunk;

				gap->prev = label->tail_gap;
				label->tail_gap = gap;
				break;
			}

			case END: 
			done = 1; 
			break;
			
			default: 
			assert(0);
			break;
		}

	}

	va_end(args);
	return 1;
}

static void node_compile(block_t *block, label_t *break_destination, label_t *continue_destination, node_t *node)
{
	
	assert(node);

	switch(node->kind) {
		case NODE_KIND_BREAK:
		node_code_starts_here(node, block);
		block_append(block, 
			U32, OPCODE_JUMP_ABSOLUTE,
			LBL, break_destination, 
			END);
		break;

		case NODE_KIND_CONTINUE:
		node_code_starts_here(node, block);
		block_append(block, 
			U32, OPCODE_JUMP_ABSOLUTE,
			LBL, continue_destination, 
			END); 
		break;

		case NODE_KIND_RETURN:
		{
			node_code_starts_here(node, block);

			node_return_t *x = (node_return_t*) node;

			node_compile(block, break_destination, continue_destination, x->expression);
			
			block_append(block, 
				U32, OPCODE_VARIABLE_MAP_POP, 
				U32, OPCODE_RETURN,
				END);
			break;
		}

		case NODE_KIND_IMPORT:
		{
			node_code_starts_here(node, block);

			node_import_t *x = (node_import_t*) node;

			node_compile(block, break_destination, continue_destination, x->expression);

			if(x->name) {

				block_append(block, 
				U32, OPCODE_IMPORT_AS, 
				STR, x->name,
				END);

			} else {

				block_append(block, 
					U32, OPCODE_IMPORT, 
					END);
			}
			break;
		}

		case NODE_KIND_IFELSE:
		{
			node_ifelse_t *x = (node_ifelse_t*) node;

			node_code_starts_here(node, block);

			if(x->else_block) {

				node_compile(block, break_destination, continue_destination, x->expression);

				label_t *label_else_start = label_create(block),
					    *label_else_end   = label_create(block);

				block_append(block, U32, OPCODE_JUMP_IF_FALSE_AND_POP, 
									LBL, label_else_start, 
									END);

				// if block start

				{
					node_compile(block, break_destination, continue_destination, x->if_block);

					if(x->if_block->kind == NODE_KIND_EXPRESSION) {

						block_append(block, 
							U32, OPCODE_POP, 
							S64, 1, END);
					}

					block_append(block, U32, OPCODE_JUMP_ABSOLUTE, 
										LBL, label_else_end, 
										END);

				}

				// if block end

				label_points_here(block, label_else_start);
				
				// else block start
				{
					node_compile(block, break_destination, continue_destination, x->else_block);

					if(x->else_block->kind == NODE_KIND_EXPRESSION)
						block_append(block, U32, OPCODE_POP, 
											S64, 1, 
											END);
				}

				// else block end

				label_points_here(block, label_else_end);

			} else {

				label_t *label_if_end = label_create(block);
					
				node_compile(block, break_destination, continue_destination, x->expression);

				block_append(block, U32, OPCODE_JUMP_IF_FALSE_AND_POP, LBL, label_if_end, END);

				// if block start

				{
					node_compile(block, break_destination, continue_destination, x->if_block);

					if(x->if_block->kind == NODE_KIND_EXPRESSION)
						block_append(block, U32, OPCODE_POP, S64, 1, END);
				}

				// if block end
				
				label_points_here(block, label_if_end);
			}

			break;
		}

		case NODE_KIND_WHILE:
		{
			node_code_starts_here(node, block);

			node_while_t *x = (node_while_t*) node;

			label_t *label_while_start = label_create(block);
			label_t *label_while_end   = label_create(block);

			label_points_here(block, label_while_start);

			node_compile(block, break_destination, continue_destination, x->expression);

			block_append(block, U32, OPCODE_JUMP_IF_FALSE_AND_POP, LBL, label_while_end, END);

			node_compile(block, label_while_end, label_while_start, x->block);

			if(x->block->kind == NODE_KIND_EXPRESSION)
				block_append(block, U32, OPCODE_POP, S64, 1);

			block_append(block, U32, OPCODE_JUMP_ABSOLUTE, LBL, label_while_start, END);

			label_points_here(block, label_while_end);

			break;
		}

		case NODE_KIND_ARGUMENT:
		case NODE_KIND_DICT_ITEM:
		assert(0);

		case NODE_KIND_EXPRESSION:
		{
			node_expr_t *x = (node_expr_t*) node;

			node_code_starts_here(node, block);

			switch(x->kind) {

				case EXPRESSION_KIND_INT:
				block_append(block, U32, OPCODE_PUSH_INT, S64, ((node_expr_int_t*) node)->value, END);
				break;

				case EXPRESSION_KIND_FLOAT:
				block_append(block, U32, OPCODE_PUSH_FLOAT, F64, ((node_expr_float_t*) node)->value, END);
				break;

				case EXPRESSION_KIND_STRING:
				block_append(block, U32, OPCODE_PUSH_STRING, STR, ((node_expr_string_t*) node)->content, END);
				break;

				case EXPRESSION_KIND_ARRAY:
				{
					node_expr_array_t *x = (node_expr_array_t*) node;

					node_t *item = x->item_head;

					int i = 0;

					while(item) {

						node_compile(block, break_destination, continue_destination, item);

						i++;
						item = item->next;
					}

					block_append(block, U32, OPCODE_BUILD_ARRAY, S64, i, END);
					break;
				}
				
				case EXPRESSION_KIND_DICT:
				{
					node_expr_dict_t *x = (node_expr_dict_t*) node;

					node_t *item = x->item_head;

					int i = 0;

					while(item) {

						node_dict_item_t *x = (node_dict_item_t*) item;

						node_compile(block, break_destination, continue_destination, x->key);
						node_compile(block, break_destination, continue_destination, x->value);

						i++;
						item = item->next;
					}

					block_append(block, U32, OPCODE_BUILD_DICT, S64, i, END);
					break;
				}

				case EXPRESSION_KIND_FUNCTION:
				{
					node_expr_function_t *x = (node_expr_function_t*) node;
					
					label_t *func_body_start = label_create(block);

					block_append(block, U32, OPCODE_PUSH_FUNCTION, LBL, func_body_start, END);

					block_t *sub_block = sub_block_create(block);

					label_points_here(sub_block, func_body_start);

					{

						node_t *argument = x->argument_head;

						node_t **arguments = alloca(sizeof(node_t*) * x->argument_count);
						char **names = alloca(sizeof(char*) * x->argument_count);
						int   i = 0;

						// Build an array of argument names

						{
							while(argument) {
								arguments[i++] = argument;
								names[i++] = ((node_argument_t*) argument)->name;
								argument = argument->next;
							}
						}

						block_append(sub_block, U32, OPCODE_EXPECT, 
												S64, x->argument_count, 
												U32, OPCODE_VARIABLE_MAP_PUSH, END);

						// Iterate it backwards

						for(int j = i-1; j >= 0; j--) {

							node_code_starts_here(arguments[j], sub_block);

							block_append(sub_block, U32, OPCODE_ASSIGN, 
													STR, names[j], 
													U32, OPCODE_POP, 
													S64, 1, END);

						}
	
						block_append(sub_block, U32, OPCODE_POP,
												S64, 1, END);
					}

					node_compile(sub_block, 0, 0, x->body);

					if(x->body->kind == NODE_KIND_EXPRESSION)
						block_append(sub_block, U32, OPCODE_POP, S64, 1, END);

					block_append(sub_block, U32, OPCODE_VARIABLE_MAP_POP,
											U32, OPCODE_PUSH_NULL,
											U32, OPCODE_RETURN, END);
					break;
				}

				case EXPRESSION_KIND_INDEX_SELECTION:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					node_expr_t *l, *r;

					l = (node_expr_t*) x->operand_head;
					r = (node_expr_t*) x->operand_tail;

					node_compile(block, break_destination, continue_destination, (node_t*) l);
					node_compile(block, break_destination, continue_destination, (node_t*) r);
					block_append(block, U32, OPCODE_SELECT, END);
					break;	
				}

				case EXPRESSION_KIND_DOT_SELECTION:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					node_expr_t *l, *r;

					l = (node_expr_t*) x->operand_head;
					r = (node_expr_t*) x->operand_tail;

					node_compile(block, break_destination, continue_destination, (node_t*) l);

					node_code_starts_here(x->operand_tail, block);
					block_append(block, U32, OPCODE_SELECT_ATTRIBUTE, 
										STR, ((node_expr_identifier_t*) r)->content, END);
					break;	
				}

				case EXPRESSION_KIND_CALL:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					int argc = x->operand_count;

					node_t *called = x->operand_head;
					node_t *arg    = called->next;

					if(((node_expr_t*) called)->kind == EXPRESSION_KIND_DOT_SELECTION) {

						node_t *container;
						node_t *identifier;

						container = ((node_expr_operation_t*) called)->operand_head;
						identifier = ((node_expr_operation_t*) called)->operand_tail;

						node_compile(block, break_destination, continue_destination, container);

						node_code_starts_here((node_t*) identifier, block);
						block_append(block, U32, OPCODE_SELECT_ATTRIBUTE_AND_REPUSH, 
											STR, ((node_expr_identifier_t*) identifier)->content, END);

						argc++;

					} else {

						node_compile(block, break_destination, continue_destination, called);
					}

					while(arg) {
						node_compile(block, break_destination, continue_destination, arg);
						arg = arg->next;
					}

					block_append(block, U32, OPCODE_CALL, S64, argc, END);
					break;	
				}

				case EXPRESSION_KIND_IDENTIFIER:
				block_append(block, U32, OPCODE_PUSH_VARIABLE, STR, ((node_expr_identifier_t*) node)->content, END);
				break;

				case EXPRESSION_KIND_NOT:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				block_append(block, U32, OPCODE_NOT, END);
				break;

				case EXPRESSION_KIND_NEG:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				block_append(block, U32, OPCODE_NEG, END);
				break;

				case EXPRESSION_KIND_BITWISE_NOT:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				block_append(block, U32, OPCODE_BITWISE_NOT, END);
				break;

				case EXPRESSION_KIND_ADD:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_ADD, END);
				break;

				case EXPRESSION_KIND_SUB:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_SUB, END);
				break;

				case EXPRESSION_KIND_MUL:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_MUL, END);
				break;

				case EXPRESSION_KIND_DIV:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_DIV, END);
				break;

				case EXPRESSION_KIND_MOD:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_MOD, END);
				break;

				case EXPRESSION_KIND_POW:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_POW, END);
				break;

				case EXPRESSION_KIND_LSS:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_LSS, END);
				break;

				case EXPRESSION_KIND_GRT:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_GRT, END);
				break;

				case EXPRESSION_KIND_LEQ:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_LEQ, END);
				break;

				case EXPRESSION_KIND_GEQ:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_GEQ, END);
				break;

				case EXPRESSION_KIND_EQL:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_EQL, END);
				break;

				case EXPRESSION_KIND_NQL:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_NQL, END);
				break;

				case EXPRESSION_KIND_AND:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_AND, END);
				break;

				case EXPRESSION_KIND_OR:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_OR, END);
				break;

				case EXPRESSION_KIND_BITWISE_AND:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_BITWISE_AND, END);
				break;

				case EXPRESSION_KIND_BITWISE_OR:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_BITWISE_OR, END);
				break;

				case EXPRESSION_KIND_BITWISE_XOR:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_BITWISE_XOR, END);
				break;

				case EXPRESSION_KIND_SHL:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_SHL, END);
				break;

				case EXPRESSION_KIND_SHR:
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_head);
				node_compile(block, break_destination, continue_destination, ((node_expr_operation_t*) node)->operand_tail);
				block_append(block, U32, OPCODE_SHR, END);
				break;


				case EXPRESSION_KIND_PRE_INC:
				case EXPRESSION_KIND_PRE_DEC:
				case EXPRESSION_KIND_POST_INC:
				case EXPRESSION_KIND_POST_DEC:
				#warning "Implement compilation of post/pre-increment/decrement operators"
				assert(0);

				case EXPRESSION_KIND_ASSIGN:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;
					
					node_expr_t *l, *r;

					l = (node_expr_t*) x->operand_head;
					r = (node_expr_t*) x->operand_tail;

					switch(l->kind) {

						case EXPRESSION_KIND_IDENTIFIER:
						{
							node_compile(block, break_destination, continue_destination, (node_t*) r);

							node_code_starts_here((node_t*) l, block);
							block_append(block, U32, OPCODE_ASSIGN, STR, ((node_expr_identifier_t*) l)->content, END);
							break;
						}

						case EXPRESSION_KIND_INDEX_SELECTION:
						{
							node_t *container, *index, *value;

							container = ((node_expr_operation_t*) l)->operand_head;
							index     = ((node_expr_operation_t*) l)->operand_tail;
							value     = (node_t*) r;

							node_compile(block, break_destination, continue_destination, container);
							node_compile(block, break_destination, continue_destination, index);
							node_compile(block, break_destination, continue_destination, value);
							block_append(block, U32, OPCODE_INSERT, END);
							break;
						}

						case EXPRESSION_KIND_DOT_SELECTION:
						{
							node_t *container, *attribute_name, *value;

							container 		= ((node_expr_operation_t*) l)->operand_head;
							attribute_name  = ((node_expr_operation_t*) l)->operand_tail;
							value     		= (node_t*) r;

							node_compile(block, break_destination, continue_destination, container);
							node_compile(block, break_destination, continue_destination, value);

							node_code_starts_here((node_t*) attribute_name, block);
							block_append(block, U32, OPCODE_INSERT_ATTRIBUTE, 
												STR, ((node_expr_identifier_t*) attribute_name)->content, END);
							break;
						}
					}
					break;
				}

				case EXPRESSION_KIND_ASSIGN_ADD:
				case EXPRESSION_KIND_ASSIGN_SUB:
				case EXPRESSION_KIND_ASSIGN_MUL:
				case EXPRESSION_KIND_ASSIGN_DIV:
				case EXPRESSION_KIND_ASSIGN_MOD:
				case EXPRESSION_KIND_ASSIGN_BITWISE_AND:
				case EXPRESSION_KIND_ASSIGN_BITWISE_OR:
				case EXPRESSION_KIND_ASSIGN_BITWISE_XOR:
				case EXPRESSION_KIND_ASSIGN_SHL:
				case EXPRESSION_KIND_ASSIGN_SHR:
				#warning "Implement assign operators compilation"
				assert(0);
				
			}
			break;
		}

		case NODE_KIND_COMPOUND:
		{
			node_compound_t *x = (node_compound_t*) node;

			node_t *stmt = x->head;

			while(stmt) {

				node_compile(block, break_destination, continue_destination, stmt);

				if(stmt->kind == NODE_KIND_EXPRESSION)
					block_append(block, U32, OPCODE_POP, S64, 1, END);

				stmt = stmt->next;
			}

			break;
		}
	}
}