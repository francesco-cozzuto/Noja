
#include <string.h>
#include <alloca.h>
#include "../noja.h"

#define FAILED //fprintf(stderr, ">> Failed at %s:%d\n", __FILE__, __LINE__);

enum {
	BLOCK_FUNC,
	BLOCK_LOOP,
};

typedef struct {

	int block_types[128];
	int block_depth;

	const char *source;
	int source_length;

	string_builder_t *output_builder;

} checking_context_t;

void find_line_range(const char *source, int source_length, int offset, int *prev_line_offset, int *line_offset, int *next_line_offset, int *line_no);
void print_line(string_builder_t *output_builder, const char *source, int source_length, int offset);

static void print_node_start_location(string_builder_t *output_builder, const char *source, int source_length, node_t *node)
{
	int prev_line_offset,
		next_line_offset,
		line_offset,
		line_no;

	find_line_range(source, source_length, node->offset, &prev_line_offset, &line_offset, &next_line_offset, &line_no);

	string_builder_append(output_builder, "\n [line] | [code]\n");

	if(prev_line_offset > -1) {

		char buffer[128];
		sprintf(buffer, "  %-5d | ", line_no - 1);

		string_builder_append(output_builder, "${zero-terminated-string}", buffer);
		print_line(output_builder, source, source_length, prev_line_offset);
		string_builder_append(output_builder, "\n");
	}

	{
		char buffer[128];
		sprintf(buffer, "  %-5d | ", line_no);

		string_builder_append(output_builder, "${zero-terminated-string}", buffer);
		print_line(output_builder, source, source_length, line_offset);
		string_builder_append(output_builder, " <- here\n");
	}

	if(next_line_offset > -1) {

		char buffer[128];
		sprintf(buffer, "  %-5d | ", line_no + 1);

		string_builder_append(output_builder, "${zero-terminated-string}", buffer);
		print_line(output_builder, source, source_length, next_line_offset);
		string_builder_append(output_builder, "\n");
	}
}

static int inside_loop(checking_context_t *ctx)
{
	for(int i = ctx->block_depth-1; i >= 0; i--) {

		if(ctx->block_types[i] == BLOCK_FUNC)
			return 0;

		if(ctx->block_types[i] == BLOCK_LOOP)
			return 1;
	}

	return 0;
}

static int inside_func(checking_context_t *ctx)
{
	for(int i = ctx->block_depth-1; i >= 0; i--) {

		if(ctx->block_types[i] == BLOCK_FUNC)
			return 1;
	}

	return 0;
}

static void push_func(checking_context_t *ctx)
{
	ctx->block_types[ctx->block_depth++] = BLOCK_FUNC;
}

static void push_loop(checking_context_t *ctx)
{
	ctx->block_types[ctx->block_depth++] = BLOCK_LOOP;
}

static void pop(checking_context_t *ctx)
{
	ctx->block_depth--;
}

static int node_check(checking_context_t *ctx, node_t *node)
{
	switch(node->kind) {

		case NODE_KIND_BREAK:
		{
			if(!inside_loop(ctx)) {

				FAILED;

				// #ERROR
				// break statement outside of a loop

				string_builder_append(ctx->output_builder, "Found break statement outside of a loop");
				print_node_start_location(ctx->output_builder, ctx->source, ctx->source_length, node);

				return 0;
			}
			return 1;
		}

		case NODE_KIND_CONTINUE:
		{
			if(!inside_loop(ctx)) {

				FAILED;

				// #ERROR
				// continue statement outside of a loop

				string_builder_append(ctx->output_builder, "Found continue statement outside of a loop");
				print_node_start_location(ctx->output_builder, ctx->source, ctx->source_length, node);

				return 0;
			}
			return 1;
		}

		case NODE_KIND_RETURN:
		{
			if(!inside_func(ctx)) {

				FAILED;

				// #ERROR
				// return statement outside of a function

				string_builder_append(ctx->output_builder, "Found return statement outsize of a function");
				print_node_start_location(ctx->output_builder, ctx->source, ctx->source_length, node);
				return 0;
			}

			node_return_t *x = (node_return_t*) node;

			if(!node_check(ctx, x->expression))
				return 0;
			return 1;
		}

		case NODE_KIND_IMPORT:
		{
			node_import_t *x = (node_import_t*) node;

			if(!node_check(ctx, x->expression))
				return 0;

			return 1;
		}

		case NODE_KIND_IFELSE:
		{
			node_ifelse_t *x = (node_ifelse_t*) node;

			if(!node_check(ctx, x->expression))
				return 0;

			if(!node_check(ctx, x->if_block))
				return 0;

			if(x->else_block)
				if(!node_check(ctx, x->else_block))
					return 0;

			return 1;
		}

		case NODE_KIND_WHILE:
		{
			node_while_t *x = (node_while_t*) node;

			if(!node_check(ctx, x->expression))
				return 0;

			push_loop(ctx);

			if(!node_check(ctx, x->block))
				return 0;

			pop(ctx);

			return 1;
		}

		case NODE_KIND_ARGUMENT:return 1;

		case NODE_KIND_DICT_ITEM:
		{
			node_dict_item_t *x = (node_dict_item_t*) node;

			if(!node_check(ctx, x->key))
				return 0;

			if(!node_check(ctx, x->value))
				return 0;

			return 1;
		}

		case NODE_KIND_EXPRESSION:
		{
			node_expr_t *x = (node_expr_t*) node;

			switch(x->kind) {
				case EXPRESSION_KIND_INT:return 1;
				case EXPRESSION_KIND_FLOAT:return 1;
				case EXPRESSION_KIND_STRING:return 1;

				case EXPRESSION_KIND_ARRAY:
				{
					node_expr_array_t *x = (node_expr_array_t*) node;

					node_t *item = x->item_head;

					while(item) {

						if(!node_check(ctx, item))
							return 0;

						item = item->next;
					}

					return 1;
				}

				case EXPRESSION_KIND_DICT:
				{
					node_expr_dict_t *x = (node_expr_dict_t*) node;

					node_t *item = x->item_head;

					while(item) {

						if(!node_check(ctx, item))
							return 0;

						item = item->next;
					}

					return 1;
				}

				case EXPRESSION_KIND_FUNCTION:
				{
					node_expr_function_t *x = (node_expr_function_t*) node;
					// check arguments

					{
						node_t *argument = x->argument_head;

						char **names = alloca(sizeof(char*) * x->argument_count);
						int    names_count = 0;

						while(argument) {

							char *name = ((node_argument_t*) argument)->name;

							for(int i = 0; i < names_count; i++) {

								if(!strcmp(name, names[i])) {

									FAILED;

									// #ERROR
									// Two arguments have the same name

									string_builder_append(ctx->output_builder, 
										"Arguments ${integer} and ${integer} have the same name \"${zero-terminated-string}\"", i, names_count, name);
									print_node_start_location(ctx->output_builder, ctx->source, ctx->source_length, argument);

									return 0;
								}
							}

							names[names_count++] = name;

							argument = argument->next;
						}

						push_func(ctx);

						if(!node_check(ctx, x->body))
							return 0;

						pop(ctx);

						return 1;
					}
				}

				case EXPRESSION_KIND_IDENTIFIER:return 1;

				case EXPRESSION_KIND_NOT:
				case EXPRESSION_KIND_NEG:
				case EXPRESSION_KIND_BITWISE_NOT:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					if(!node_check(ctx, x->operand_head))
						return 0;
					
					return 1;
				}

				case EXPRESSION_KIND_CALL:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					node_t *arg = x->operand_head;

					while(arg) {

						if(!node_check(ctx, arg))
							return 0;

						arg = arg->next;
					}

					return 1;
				}

				case EXPRESSION_KIND_ADD:
				case EXPRESSION_KIND_SUB:
				case EXPRESSION_KIND_MUL:
				case EXPRESSION_KIND_DIV:
				case EXPRESSION_KIND_MOD:
				case EXPRESSION_KIND_POW:
				case EXPRESSION_KIND_LSS:
				case EXPRESSION_KIND_GRT:
				case EXPRESSION_KIND_LEQ:
				case EXPRESSION_KIND_GEQ:
				case EXPRESSION_KIND_EQL:
				case EXPRESSION_KIND_NQL:
				case EXPRESSION_KIND_AND:
				case EXPRESSION_KIND_OR:
				case EXPRESSION_KIND_BITWISE_AND:
				case EXPRESSION_KIND_BITWISE_OR:
				case EXPRESSION_KIND_BITWISE_XOR:
				case EXPRESSION_KIND_SHL:
				case EXPRESSION_KIND_SHR:
				case EXPRESSION_KIND_DOT_SELECTION:
				case EXPRESSION_KIND_INDEX_SELECTION:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					if(!node_check(ctx, x->operand_head))
						return 0;

					if(!node_check(ctx, x->operand_tail))
						return 0;
					
					return 1;
				}

				case EXPRESSION_KIND_PRE_INC:
				case EXPRESSION_KIND_PRE_DEC:
				case EXPRESSION_KIND_POST_INC:
				case EXPRESSION_KIND_POST_DEC:
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					if(!node_check(ctx, x->operand_head))
						return 0;

					node_expr_t *l = (node_expr_t*) x->operand_head;

					if(l->kind != EXPRESSION_KIND_IDENTIFIER) {

						FAILED;

						// #ERROR
						// Increment or decrement of something that is not a variable
						
						string_builder_append(ctx->output_builder, "Found increment or decrement of something that is not a variable");
						print_node_start_location(ctx->output_builder, ctx->source, ctx->source_length, node);
						return 0;
					}
					
					return 1;
				}

				case EXPRESSION_KIND_ASSIGN:
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
				{
					node_expr_operation_t *x = (node_expr_operation_t*) node;

					if(!node_check(ctx, x->operand_head))
						return 0;

					if(!node_check(ctx, x->operand_tail))
						return 0;

					node_expr_t *l = (node_expr_t*) x->operand_head;

					if(l->kind != EXPRESSION_KIND_IDENTIFIER && l->kind != EXPRESSION_KIND_INDEX_SELECTION && l->kind != EXPRESSION_KIND_DOT_SELECTION) {
						FAILED;

						// #ERROR
						// Assignment to something that is not a assignable

						string_builder_append(ctx->output_builder, "Found increment or decrement of something that is not a variable");
						print_node_start_location(ctx->output_builder, ctx->source, ctx->source_length, node);
						return 0;
					}

					return 1;
				}
				
			}
			assert(0);
			return 0;
		}

		case NODE_KIND_COMPOUND:
		{
			node_compound_t *x = (node_compound_t*) node;

			node_t *stmt = x->head;

			while(stmt) {

				if(!node_check(ctx, stmt))
					return 0;

				stmt = stmt->next;
			}

			return 1;
		}
	}

	assert(0);
	return 0;
}

int check(node_t *node, const char *source, int source_length, string_builder_t *output_builder)
{
	checking_context_t ctx;

	ctx.source = source;
	ctx.source_length = source_length;
	ctx.block_depth = 0;
	ctx.output_builder = output_builder;

	if(!node_check(&ctx, node))
		return 0;

	return 1;
}