
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
#include "noja.h"

#warning "Align data in code generation"

#define BYTES_PER_DATA_CHUNK 4096
#define BYTES_PER_CODE_CHUNK 1024

typedef struct function_text_t function_text_t;
typedef struct data_chunk_t data_chunk_t;
typedef struct offset_gap_t offset_gap_t;
typedef struct label_t label_t;
typedef struct function_text_chunk_t function_text_chunk_t;
typedef struct generating_context_t generating_context_t;

struct label_t {
	function_text_t *function_text;
	function_text_chunk_t *chunk;
	uint32_t chunk_offset;
	uint32_t offset;
};

struct offset_gap_t {
	offset_gap_t *prev;
	label_t refering, 
			referred;
};

struct data_chunk_t {
	data_chunk_t *next;
	uint32_t used;
	char content[BYTES_PER_DATA_CHUNK];
};

struct function_text_chunk_t {

	function_text_chunk_t *next;
	
	uint32_t used;
	char content[BYTES_PER_CODE_CHUNK];
};

struct function_text_t {
	
	generating_context_t *context;
	
	function_text_t *next;

	function_text_chunk_t head, 
						 *tail;
	
	uint32_t offset, 
			 length;
};

struct generating_context_t {

	data_chunk_t head_data, *tail_data;
	int data_length;

	function_text_t head_function, 
				   *tail_function;

	offset_gap_t *tail_call_gap;

	jmp_buf env;
};

static void throw(generating_context_t *ctx)
{
	longjmp(ctx->env, 1);
}

label_t function_text_get_label_here(function_text_t *function_text)
{
	return (label_t) { .function_text = function_text, .chunk = function_text->tail, .chunk_offset = function_text->tail->used, .offset = function_text->length };
}

void ensure_space(function_text_t *function_text, uint32_t bytes)
{
	if(function_text->tail->used + bytes <= BYTES_PER_CODE_CHUNK)
		return;

	// NOTE: bytes can never be greater than BYTES_PER_CODE_CHUNK

	function_text_chunk_t *chunk = malloc(sizeof(function_text_chunk_t));

	if(chunk == 0)
		throw(function_text->context);

	chunk->next = 0;
	chunk->used = 0;
	function_text->tail->next = chunk;
	function_text->tail = chunk;
}

void function_text_append_u32(function_text_t *function_text, uint32_t value)
{
	ensure_space(function_text, sizeof(uint32_t));

	*(uint32_t*) (function_text->tail->content + function_text->tail->used) = value;

	function_text->tail->used += sizeof(uint32_t);
	function_text->length += sizeof(uint32_t);		
}

void function_text_append_i64(function_text_t *function_text, int64_t value)
{
	ensure_space(function_text, sizeof(int64_t));

	*(int64_t*) (function_text->tail->content + function_text->tail->used) = value;

	function_text->tail->used += sizeof(int64_t);
	function_text->length += sizeof(int64_t);
}

void function_text_append_f64(function_text_t *function_text, double value)
{
	ensure_space(function_text, sizeof(double));

	*(double*) (function_text->tail->content + function_text->tail->used) = value;

	function_text->tail->used += sizeof(double);
	function_text->length += sizeof(double);
}

void function_text_append_string(function_text_t *function_text, const char *value)
{
	// write the string to the data segment and get its offset

	uint32_t offset;

	{
		generating_context_t *context = function_text->context;

		offset = context->data_length;

		char c;
		int i = 0;

		do {
	
			if(context->tail_data->used + 1 == BYTES_PER_DATA_CHUNK) {

				data_chunk_t *chunk = malloc(sizeof(data_chunk_t));

				if(chunk == 0)	
					throw(context);

				chunk->next = 0;
				chunk->used = 0;
	
				context->tail_data->next = chunk;
				context->tail_data = chunk;
			}

			c = value[i++];
	
			context->tail_data->content[context->tail_data->used++] = c;
			context->data_length++;

		} while(c != '\0');
	}

	// write the offset to the data segment into the function body

	ensure_space(function_text, sizeof(uint32_t));

	*(uint32_t*) (function_text->tail->content + function_text->tail->used) = offset;

	function_text->tail->used += sizeof(uint32_t);
	function_text->length += sizeof(uint32_t);		
}

void function_text_write_u32(label_t label, uint32_t value)
{
	*(uint32_t*) (label.chunk->content + label.chunk_offset) = value;
}

void function_text_write_i64(label_t label, int64_t value)
{
	*(int64_t*) (label.chunk->content + label.chunk_offset) = value;
}

void function_text_write_f64(label_t label, double value)
{
	*(double*) (label.chunk->content + label.chunk_offset) = value;
}

void function_text_write_u32_from_label(label_t label, label_t referred)
{
	offset_gap_t *gap = malloc(sizeof(offset_gap_t));

	if(gap == 0)
		throw(label.function_text->context);

	gap->refering = label;
	gap->referred = referred;

	gap->prev = label.function_text->context->tail_call_gap;
	label.function_text->context->tail_call_gap = gap;
}

function_text_t *function_text_create(generating_context_t *ctx)
{
	// create

	function_text_t *ft = malloc(sizeof(function_text_t));

	if(ft == 0)
		throw(ctx);

	// initialize

	ft->context = ctx;
	ft->next = 0;
	ft->head.next = 0;
	ft->head.used = 0;
	ft->tail = &ft->head;
	ft->length = 0;

	// append to the context

	ctx->tail_function->next = ft;
	ctx->tail_function = ft;

	return ft;
}

static void release_resources(generating_context_t ctx)
{
	// free gaps
	{
		offset_gap_t *gap = ctx.tail_call_gap;

		while(gap) {

			offset_gap_t *prev_gap = gap->prev;

			free(gap);

			gap = prev_gap;
		}

		ctx.tail_call_gap = 0;
	}

	// free data chunks

	{
		data_chunk_t *chunk = ctx.head_data.next;

		while(chunk) {

			data_chunk_t *next_chunk = chunk->next;

			free(chunk);

			chunk = next_chunk;
		}
	}

	// free functions

	{

		function_text_t *ft = &ctx.head_function;
		int i = 0;

		while(ft) {

			// free function

			function_text_t *next_ft = ft->next;

			{
				// free chunks

				function_text_chunk_t *c = ft->head.next;
			
				while(c) {

					function_text_chunk_t *next_c = c->next;

					free(c);

					c = next_c;
				}
			}

			// free the function if it's not the first one

			if(i > 0)
				free(ft);

			ft = next_ft;
			i++;
		}
	}
}

static void node_compile(function_text_t *ft, node_t *node);

int generate(ast_t ast, char **e_data, char **e_code, uint32_t *e_data_size, uint32_t *e_code_size)
{
	generating_context_t ctx;
	ctx.head_data.next = 0;
	ctx.head_data.used = 0;
	ctx.tail_data = &ctx.head_data;
	ctx.data_length = 0;
	ctx.head_function.context = &ctx;
	ctx.head_function.next = 0;
	ctx.head_function.head.next = 0;
	ctx.head_function.head.used = 0;
	ctx.head_function.tail = &ctx.head_function.head;
	ctx.head_function.length = 0;
	ctx.tail_function = &ctx.head_function;
	ctx.tail_call_gap = 0;

	if(setjmp(ctx.env)) {

		release_resources(ctx);
		return 0;
	}

	node_compile(&ctx.head_function, ast.root);

	function_text_append_u32(&ctx.head_function, OPCODE_QUIT);

	// set function offsets and determine the whole code segment length

	uint32_t code_length = 0;

	{
		function_text_t *ft = &ctx.head_function;

		while(ft) {

			ft->offset = code_length;
			code_length += ft->length;

			ft = ft->next;
		}
	}

	// fill address gaps

	{
		offset_gap_t *gap = ctx.tail_call_gap;

		while(gap) {

			offset_gap_t *prev_gap = gap->prev;

			*(uint32_t*) (gap->refering.chunk->content + gap->refering.chunk_offset) = gap->referred.function_text->offset + gap->referred.offset; 

			free(gap);

			gap = prev_gap;
		}

		ctx.tail_call_gap = 0;
	}

	char *code = malloc(ctx.data_length + code_length);
	char *data = code + code_length;

	if(code == 0)
		throw(&ctx);
	

	// write the data

	{
		uint32_t written;

		memcpy(data, ctx.head_data.content, ctx.head_data.used);

		written = ctx.head_data.used;

		data_chunk_t *chunk = ctx.head_data.next;

		while(chunk) {

			data_chunk_t *next_chunk = chunk->next;

			memcpy(data + written, chunk->content, chunk->used);

			written += chunk->used;

			free(chunk);

			chunk = next_chunk;
		}
	}


	// serialize code

	{
		uint32_t written = 0;
		uint32_t i = 0;

		function_text_t *ft = &ctx.head_function;
	
		while(ft) {

			// serialize the function

			function_text_t *next_ft = ft->next;

			{
				// serialize the first function chunk

				memcpy(code + written, ft->head.content, ft->head.used);
				written += ft->head.used;

				// serialize the remanining function chunks

				function_text_chunk_t *c = ft->head.next;
			
				while(c) {

					function_text_chunk_t *next_c = c->next;

					memcpy(code + written, c->content, c->used);
					written += c->used;

					free(c);

					c = next_c;
				}
			}

			// free the function if it's not the first one

			if(i > 0)
				free(ft);

			ft = next_ft;
			i++;
		}
	}

	*e_code = code;
	*e_data = data;
	*e_code_size = code_length;
	*e_data_size = ctx.data_length;

	return 1;
}


static void node_compile(function_text_t *ft, node_t *node)
{
	
	assert(node);

	switch(node->kind) {
		case NODE_KIND_BREAK: function_text_append_u32(ft, OPCODE_BREAK); break;
		case NODE_KIND_CONTINUE: function_text_append_u32(ft, OPCODE_CONTINUE); break;

		case NODE_KIND_RETURN:
		{
			node_return_t *x = (node_return_t*) node;

			node_compile(ft, x->expression);
			function_text_append_u32(ft, OPCODE_VARIABLE_MAP_POP);
			function_text_append_u32(ft, OPCODE_RETURN);
			break;
		}

		case NODE_KIND_IFELSE:
		{
			node_ifelse_t *x = (node_ifelse_t*) node;

			if(x->else_block) {

				node_compile(ft, x->expression);

				function_text_append_u32(ft, OPCODE_JUMP_IF_FALSE_AND_POP);

				label_t A = function_text_get_label_here(ft);

				function_text_append_u32(ft, 0);

				node_compile(ft, x->if_block);

				if(x->if_block->kind == NODE_KIND_EXPRESSION) {
					function_text_append_u32(ft, OPCODE_POP);
					function_text_append_i64(ft, 1);
				}

				function_text_append_u32(ft, OPCODE_JUMP_ABSOLUTE);

				label_t B = function_text_get_label_here(ft);

				function_text_append_u32(ft, 0);

				label_t C = function_text_get_label_here(ft);

				function_text_write_u32_from_label(A, C);

				node_compile(ft, x->else_block);

				if(x->else_block->kind == NODE_KIND_EXPRESSION) {
					function_text_append_u32(ft, OPCODE_POP);
					function_text_append_i64(ft, 1);
				}

				function_text_write_u32_from_label(B, C);

			} else {

				node_compile(ft, x->expression);

				function_text_append_u32(ft, OPCODE_JUMP_IF_FALSE_AND_POP);

				label_t A = function_text_get_label_here(ft);

				function_text_append_u32(ft, 0);

				node_compile(ft, x->if_block);

				if(x->if_block->kind == NODE_KIND_EXPRESSION) {
					function_text_append_u32(ft, OPCODE_POP);
					function_text_append_i64(ft, 1);
				}

				label_t B = function_text_get_label_here(ft);

				function_text_write_u32_from_label(A, B);
			}

			break;
		}

		case NODE_KIND_WHILE:
		{
			node_while_t *x = (node_while_t*) node;

			function_text_append_u32(ft, OPCODE_CONTINUE_DESTINATION_PUSH);

			label_t A = function_text_get_label_here(ft);
			function_text_append_u32(ft, 0);

			function_text_append_u32(ft, OPCODE_BREAK_DESTINATION_PUSH);

			label_t B = function_text_get_label_here(ft);
			function_text_append_u32(ft, 0);

			label_t Q = function_text_get_label_here(ft);
			function_text_write_u32_from_label(A, Q);

			label_t G = function_text_get_label_here(ft);

			node_compile(ft, x->expression);

			function_text_append_u32(ft, OPCODE_JUMP_IF_FALSE_AND_POP);

			label_t C = function_text_get_label_here(ft);
			function_text_append_u32(ft, 0);

			#warning "Handle the case where a while-block or if-else block returns, since the continue and break stacks need to be popped"
			node_compile(ft, x->block); // #TODO Handle the case where this block returns! The continue and break destinations need to be popped!

			if(x->block->kind == NODE_KIND_EXPRESSION) {
				function_text_append_u32(ft, OPCODE_POP);
				function_text_append_i64(ft, 1);
			}

			function_text_append_u32(ft, OPCODE_JUMP_ABSOLUTE);
			
			label_t T = function_text_get_label_here(ft);
			function_text_append_u32(ft, 0);

			function_text_write_u32_from_label(T, G);

			label_t D = function_text_get_label_here(ft);

			function_text_write_u32_from_label(B, D);
			function_text_write_u32_from_label(C, D);

			function_text_append_u32(ft, OPCODE_CONTINUE_DESTINATION_POP);
			function_text_append_u32(ft, OPCODE_BREAK_DESTINATION_POP);
			break;
		}

		case NODE_KIND_ARGUMENT:
		case NODE_KIND_DICT_ITEM:
		assert(0);

		case NODE_KIND_EXPRESSION:
		{
			node_expr_t *x = (node_expr_t*) node;

			switch(x->kind) {

				case EXPRESSION_KIND_INT:
				function_text_append_u32(ft, OPCODE_PUSH_INT);
				function_text_append_i64(ft, ((node_expr_int_t*) node)->value); // #TODO Should align
				break;

				case EXPRESSION_KIND_FLOAT:
				function_text_append_u32(ft, OPCODE_PUSH_FLOAT);
				function_text_append_f64(ft, ((node_expr_float_t*) node)->value); // #TODO Should align
				break;

				case EXPRESSION_KIND_STRING:
				function_text_append_u32(ft, OPCODE_PUSH_STRING);
				function_text_append_string(ft, ((node_expr_string_t*) node)->content);
				break;

				case EXPRESSION_KIND_ARRAY:
				{
					node_expr_array_t *x = (node_expr_array_t*) node;

					node_t *item = x->item_head;

					int i = 0;

					while(item) {

						node_compile(ft, item);

						i++;
						item = item->next;
					}

					function_text_append_u32(ft, OPCODE_BUILD_ARRAY);
					function_text_append_i64(ft, i);
					break;
				}
				
				case EXPRESSION_KIND_DICT:
				{
					node_expr_dict_t *x = (node_expr_dict_t*) node;

					node_t *item = x->item_head;

					int i = 0;

					while(item) {

						node_dict_item_t *x = (node_dict_item_t*) item;

						node_compile(ft, x->key);
						node_compile(ft, x->value);

						i++;
						item = item->next;
					}

					function_text_append_u32(ft, OPCODE_BUILD_DICT);
					function_text_append_i64(ft, i);
					break;
				}

				case EXPRESSION_KIND_FUNCTION:
				{
					node_expr_function_t *x = (node_expr_function_t*) node;
					
					function_text_append_u32(ft, OPCODE_PUSH_FUNCTION);

					function_text_t *sub_ft = function_text_create(ft->context);

					label_t A, B;

					A = function_text_get_label_here(ft);
					B = function_text_get_label_here(sub_ft);

					function_text_append_u32(ft, 0);

					function_text_write_u32_from_label(A, B);

					{

						node_t *argument = x->argument_head;

						char **names = alloca(sizeof(char*) * x->argument_count);
						int   i = 0;

						// Build an array of argument names

						{
							while(argument) {
								names[i++] = ((node_argument_t*) argument)->name;
								argument = argument->next;
							}
						}

						function_text_append_u32(sub_ft, OPCODE_EXPECT);
						function_text_append_i64(sub_ft, x->argument_count);

						function_text_append_u32(sub_ft, OPCODE_VARIABLE_MAP_PUSH);

						// Iterate it backwards

						for(int j = i-1; j >= 0; j--) {

							function_text_append_u32(sub_ft, OPCODE_ASSIGN);
							function_text_append_string(sub_ft, names[j]);

							function_text_append_u32(sub_ft, OPCODE_POP);
							function_text_append_i64(sub_ft, 1);
						}

						function_text_append_u32(sub_ft, OPCODE_POP);
						function_text_append_i64(sub_ft, 1);
					}

					node_compile(sub_ft, x->body);

					if(x->body->kind == NODE_KIND_EXPRESSION) {
						function_text_append_u32(ft, OPCODE_POP);
						function_text_append_i64(ft, 1);
					}

					function_text_append_u32(sub_ft, OPCODE_VARIABLE_MAP_POP);
					function_text_append_u32(sub_ft, OPCODE_PUSH_NULL);
					function_text_append_u32(sub_ft, OPCODE_RETURN);
					break;
				}

				case EXPRESSION_KIND_INDEX_SELECTION:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					node_expr_t *l, *r;

					l = (node_expr_t*) x->operand_head;
					r = (node_expr_t*) x->operand_tail;

					node_compile(ft, (node_t*) l);
					node_compile(ft, (node_t*) r);
					function_text_append_u32(ft, OPCODE_SELECT);
					break;	
				}

				case EXPRESSION_KIND_DOT_SELECTION:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					node_expr_t *l, *r;

					l = (node_expr_t*) x->operand_head;
					r = (node_expr_t*) x->operand_tail;

					node_compile(ft, (node_t*) l);
					function_text_append_u32(ft, OPCODE_SELECT_ATTRIBUTE);
					function_text_append_string(ft, ((node_expr_identifier_t*) r)->content);
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

						node_compile(ft, container);
						function_text_append_u32(ft, OPCODE_SELECT_ATTRIBUTE_AND_REPUSH);
						function_text_append_string(ft, ((node_expr_identifier_t*) identifier)->content);

						argc++;

					} else {

						node_compile(ft, called);
					}

					while(arg) {
						node_compile(ft, arg);
						arg = arg->next;
					}

					function_text_append_u32(ft, OPCODE_CALL);
					function_text_append_i64(ft, argc);
					break;	
				}

				case EXPRESSION_KIND_IDENTIFIER:
				function_text_append_u32(ft, OPCODE_PUSH_VARIABLE);
				function_text_append_string(ft, ((node_expr_identifier_t*) node)->content);
				break;

				case EXPRESSION_KIND_NOT:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				function_text_append_u32(ft, OPCODE_NOT);
				break;

				case EXPRESSION_KIND_NEG:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				function_text_append_u32(ft, OPCODE_NEG);
				break;

				case EXPRESSION_KIND_BITWISE_NOT:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				function_text_append_u32(ft, OPCODE_BITWISE_NOT);
				break;

				case EXPRESSION_KIND_ADD:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_ADD);
				break;

				case EXPRESSION_KIND_SUB:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_SUB);
				break;

				case EXPRESSION_KIND_MUL:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_MUL);
				break;

				case EXPRESSION_KIND_DIV:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_DIV);
				break;

				case EXPRESSION_KIND_MOD:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_MOD);
				break;

				case EXPRESSION_KIND_POW:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_POW);
				break;

				case EXPRESSION_KIND_LSS:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_LSS);
				break;

				case EXPRESSION_KIND_GRT:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_GRT);
				break;

				case EXPRESSION_KIND_LEQ:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_LEQ);
				break;

				case EXPRESSION_KIND_GEQ:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_GEQ);
				break;

				case EXPRESSION_KIND_EQL:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_EQL);
				break;

				case EXPRESSION_KIND_NQL:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_NQL);
				break;

				case EXPRESSION_KIND_AND:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_AND);
				break;

				case EXPRESSION_KIND_OR:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_OR);
				break;

				case EXPRESSION_KIND_BITWISE_AND:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_BITWISE_AND);
				break;

				case EXPRESSION_KIND_BITWISE_OR:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_BITWISE_OR);
				break;

				case EXPRESSION_KIND_BITWISE_XOR:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_BITWISE_XOR);
				break;

				case EXPRESSION_KIND_SHL:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_SHL);
				break;

				case EXPRESSION_KIND_SHR:
				node_compile(ft, ((node_expr_operation_t*) node)->operand_head);
				node_compile(ft, ((node_expr_operation_t*) node)->operand_tail);
				function_text_append_u32(ft, OPCODE_SHR);
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
							node_compile(ft, (node_t*) r);

							function_text_append_u32(ft, OPCODE_ASSIGN);
							function_text_append_string(ft, ((node_expr_identifier_t*) l)->content);
							break;
						}

						case EXPRESSION_KIND_INDEX_SELECTION:
						{
							node_t *container, *index, *value;

							container = ((node_expr_operation_t*) l)->operand_head;
							index     = ((node_expr_operation_t*) l)->operand_tail;
							value     = (node_t*) r;

							node_compile(ft, container);
							node_compile(ft, index);
							node_compile(ft, value);
							function_text_append_u32(ft, OPCODE_INSERT);
							break;
						}

						case EXPRESSION_KIND_DOT_SELECTION:
						{
							node_t *container, *attribute_name, *value;

							container 		= ((node_expr_operation_t*) l)->operand_head;
							attribute_name  = ((node_expr_operation_t*) l)->operand_tail;
							value     		= (node_t*) r;

							node_compile(ft, container);
							node_compile(ft, value);
							function_text_append_u32(ft, OPCODE_INSERT_ATTRIBUTE);
							function_text_append_string(ft, ((node_expr_identifier_t*) attribute_name)->content);
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

				node_compile(ft, stmt);

				if(stmt->kind == NODE_KIND_EXPRESSION) {

					function_text_append_u32(ft, OPCODE_POP);
					function_text_append_i64(ft, 1);

				}

				stmt = stmt->next;
			}

			break;
		}
	}
}