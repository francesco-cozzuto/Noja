
#include <assert.h>
#include "ast.h"

void ast_delete(ast_t ast)
{
	pool_destroy(ast.pool);
}

static node_t *node_operation_create(pool_t *pool, int offset, int length, int kind, node_t *operand_head, node_t *operand_tail, int operand_count)
{
	node_expr_operation_t *node = pool_request(pool, sizeof(node_expr_operation_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = kind;
	node->operand_head = operand_head;
	node->operand_tail = operand_tail;
	node->operand_count = operand_count;

	return (node_t*) node;
}

static node_t *node_unary_operation_create(pool_t *pool, int offset, int length, int kind, node_t *operand)
{
	return node_operation_create(pool, offset, length, kind, operand, operand, 1);
}

static node_t *node_binary_operation_create(pool_t *pool, int offset, int length, int kind, node_t *left_operand, node_t *right_operand)
{
	left_operand->next = right_operand;

	return node_operation_create(pool, offset, length, kind, left_operand, right_operand, 2);
}

node_t *node_neg_create(pool_t *pool, int offset, int length, node_t *operand)
{
	return node_unary_operation_create(pool, offset, length, EXPRESSION_KIND_NEG, operand);
}

node_t *node_add_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ADD, left_operand, right_operand);
}

node_t *node_sub_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_SUB, left_operand, right_operand);
}

node_t *node_mul_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_MUL, left_operand, right_operand);
}

node_t *node_div_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_DIV, left_operand, right_operand);
}

node_t *node_mod_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_MOD, left_operand, right_operand);
}

node_t *node_pow_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_POW, left_operand, right_operand);
}

node_t *node_lss_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_LSS, left_operand, right_operand);
}

node_t *node_grt_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_GRT, left_operand, right_operand);
}

node_t *node_leq_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_LEQ, left_operand, right_operand);
}

node_t *node_geq_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_GEQ, left_operand, right_operand);
}

node_t *node_eql_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_EQL, left_operand, right_operand);
}

node_t *node_nql_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_NQL, left_operand, right_operand);
}

node_t *node_and_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_AND, left_operand, right_operand);
}

node_t *node_or_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_OR, left_operand, right_operand);
}

node_t *node_not_create(pool_t *pool, int offset, int length, node_t *operand)
{
	return node_unary_operation_create(pool, offset, length, EXPRESSION_KIND_NOT, operand);
}

node_t *node_bitwise_and_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_BITWISE_AND, left_operand, right_operand);
}

node_t *node_bitwise_or_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_BITWISE_OR, left_operand, right_operand);
}

node_t *node_bitwise_xor_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_BITWISE_XOR, left_operand, right_operand);
}

node_t *node_bitwise_not_create(pool_t *pool, int offset, int length, node_t *operand)
{
	return node_unary_operation_create(pool, offset, length, EXPRESSION_KIND_BITWISE_NOT, operand);
}

node_t *node_shl_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_SHL, left_operand, right_operand);
}

node_t *node_shr_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_SHR, left_operand, right_operand);
}

node_t *node_post_inc_create(pool_t *pool, int offset, int length, node_t *operand)
{
	return node_unary_operation_create(pool, offset, length, EXPRESSION_KIND_POST_INC, operand);
}

node_t *node_post_dec_create(pool_t *pool, int offset, int length, node_t *operand)
{
	return node_unary_operation_create(pool, offset, length, EXPRESSION_KIND_POST_DEC, operand);
}

node_t *node_pre_inc_create(pool_t *pool, int offset, int length, node_t *operand)
{
	return node_unary_operation_create(pool, offset, length, EXPRESSION_KIND_PRE_INC, operand);
}

node_t *node_pre_dec_create(pool_t *pool, int offset, int length, node_t *operand)
{
	return node_unary_operation_create(pool, offset, length, EXPRESSION_KIND_PRE_DEC, operand);
}

node_t *node_assign_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN, left_operand, right_operand);
}

node_t *node_assign_add_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_ADD, left_operand, right_operand);
}

node_t *node_assign_sub_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_SUB, left_operand, right_operand);
}

node_t *node_assign_mul_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_MUL, left_operand, right_operand);
}

node_t *node_assign_div_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_DIV, left_operand, right_operand);
}

node_t *node_assign_mod_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_MOD, left_operand, right_operand);
}

node_t *node_assign_bitwise_and_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_BITWISE_AND, left_operand, right_operand);
}

node_t *node_assign_bitwise_or_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_BITWISE_OR, left_operand, right_operand);
}

node_t *node_assign_bitwise_xor_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_BITWISE_XOR, left_operand, right_operand);
}

node_t *node_assign_shl_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_SHL, left_operand, right_operand);
}

node_t *node_assign_shr_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_ASSIGN_SHR, left_operand, right_operand);
}

node_t *node_index_selection_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_INDEX_SELECTION, left_operand, right_operand);
}

node_t *node_dot_selection_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand)
{
	return node_binary_operation_create(pool, offset, length, EXPRESSION_KIND_DOT_SELECTION, left_operand, right_operand);
}

node_t *node_call_create(pool_t *pool, int offset, int length, node_t *arg_head, node_t *arg_tail, int argc)
{
	return node_operation_create(pool, offset, length, EXPRESSION_KIND_CALL, arg_head, arg_tail, argc);
}

node_t *node_int_create(pool_t *pool, int offset, int length, int64_t value)
{
	node_expr_int_t *node = pool_request(pool, sizeof(node_expr_int_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = EXPRESSION_KIND_INT;
	node->value = value;

	return (node_t*) node;
}

node_t *node_float_create(pool_t *pool, int offset, int length, double value)
{
	node_expr_float_t *node = pool_request(pool, sizeof(node_expr_float_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = EXPRESSION_KIND_INT;
	node->value = value;

	return (node_t*) node;
}

node_t *node_string_create(pool_t *pool, int offset, int length, char *content, int content_length)
{
	node_expr_string_t *node = pool_request(pool, sizeof(node_expr_string_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = EXPRESSION_KIND_STRING;
	node->content = content;
	node->length = content_length;

	return (node_t*) node;
}

node_t *node_identifier_create(pool_t *pool, int offset, int length, char *content, int content_length)
{
	node_expr_identifier_t *node = pool_request(pool, sizeof(node_expr_identifier_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = EXPRESSION_KIND_IDENTIFIER;
	node->content = content;
	node->length = content_length;

	return (node_t*) node;
}

node_t *node_argument_create(pool_t *pool, int offset, int length, char *name)
{
	node_argument_t *node = pool_request(pool, sizeof(node_argument_t));

	if(node == 0)
		return 0;

	node->super.kind = NODE_KIND_ARGUMENT;
	node->super.offset = offset;
	node->super.length = length;
	node->super.next = 0;
	node->name = name;

	return (node_t*) node;
}

node_t *node_function_create(pool_t *pool, int offset, int length, node_t *argument_head, node_t *argument_tail, int argument_count, node_t *body)
{
	node_expr_function_t *node = pool_request(pool, sizeof(node_expr_function_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = EXPRESSION_KIND_FUNCTION;
	node->argument_head = argument_head;
	node->argument_tail = argument_tail;
	node->argument_count = argument_count;
	node->body = body;

	return (node_t*) node;
}

node_t *node_dict_item_create(pool_t *pool, int offset, int length, node_t *key, node_t *value)
{
	node_dict_item_t *node = pool_request(pool, sizeof(node_dict_item_t));

	if(node == 0)
		return 0;

	node->super.kind = NODE_KIND_DICT_ITEM;
	node->super.offset = offset;
	node->super.length = length;
	node->super.next = 0;
	node->key = key;
	node->value = value;

	return (node_t*) node;
}

node_t *node_dict_create(pool_t *pool, int offset, int length, node_t *item_head, node_t *item_tail, int item_count)
{
	node_expr_dict_t *node = pool_request(pool, sizeof(node_expr_dict_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = EXPRESSION_KIND_DICT;
	node->item_head = item_head;
	node->item_tail = item_tail;
	node->item_count = item_count;

	return (node_t*) node;
}

node_t *node_array_create(pool_t *pool, int offset, int length, node_t *item_head, node_t *item_tail, int item_count)
{
	node_expr_array_t *node = pool_request(pool, sizeof(node_expr_array_t));

	if(node == 0)
		return 0;

	node->super.super.kind = NODE_KIND_EXPRESSION;
	node->super.super.offset = offset;
	node->super.super.length = length;
	node->super.super.next = 0;
	node->super.kind = EXPRESSION_KIND_ARRAY;
	node->item_head = item_head;
	node->item_tail = item_tail;
	node->item_count = item_count;

	return (node_t*) node;
}

node_t *node_while_create(pool_t *pool, int offset, int length, node_t *expression, node_t *block)
{
	node_while_t *node = pool_request(pool, sizeof(node_while_t));

	if(node == 0)
		return 0;

	node->super.kind = NODE_KIND_WHILE;
	node->super.offset = offset;
	node->super.length = length;
	node->super.next = 0;
	node->expression = expression;
	node->block = block;

	return (node_t*) node;
}

node_t *node_ifelse_create(pool_t *pool, int offset, int length, node_t *expression, node_t *if_block, node_t *else_block)
{
	node_ifelse_t *node = pool_request(pool, sizeof(node_ifelse_t));

	if(node == 0)
		return 0;

	node->super.kind = NODE_KIND_IFELSE;
	node->super.offset = offset;
	node->super.length = length;
	node->super.next = 0;
	node->expression = expression;
	node->if_block = if_block;
	node->else_block = else_block;

	return (node_t*) node;
}

node_t *node_return_create(pool_t *pool, int offset, int length, node_t *expression)
{
	node_return_t *node = pool_request(pool, sizeof(node_return_t));

	if(node == 0)
		return 0;

	node->super.kind = NODE_KIND_RETURN;
	node->super.offset = offset;
	node->super.length = length;
	node->super.next = 0;
	node->expression = expression;


	return (node_t*) node;
}

node_t *node_break_create(pool_t *pool, int offset, int length)
{
	node_t *node = pool_request(pool, sizeof(node_t));

	if(node == 0)
		return 0;

	node->kind = NODE_KIND_BREAK;
	node->offset = offset;
	node->length = length;
	node->next = 0;

	return node;
}

node_t *node_continue_create(pool_t *pool, int offset, int length)
{
	node_t *node = pool_request(pool, sizeof(node_t));

	if(node == 0)
		return 0;

	node->kind = NODE_KIND_CONTINUE;
	node->offset = offset;
	node->length = length;
	node->next = 0;

	return node;
}

node_t *node_compound_create(pool_t *pool, int offset, int length, node_t *head, node_t *tail, int count)
{
	node_compound_t *node = pool_request(pool, sizeof(node_compound_t));

	if(node == 0)
		return 0;

	node->super.kind = NODE_KIND_COMPOUND;
	node->super.offset = offset;
	node->super.length = length;
	node->super.next = 0;
	node->head = head;
	node->tail = tail;
	node->count = count;

	return (node_t*) node;
}

node_t *node_import_create(pool_t *pool, int offset, int length, node_t *expression, char *name)
{
	node_import_t *node = pool_request(pool, sizeof(node_import_t));

	if(node == 0)
		return 0;

	node->super.kind = NODE_KIND_IMPORT;
	node->super.offset = offset;
	node->super.length = length;
	node->super.next = 0;
	node->expression = expression;
	node->name = name;

	return (node_t*) node;
}

/* =========================== */
/* === Getters and setters === */
/* =========================== */

void node_get_int(node_t *node, int64_t *e_value)
{
	assert(node->kind == NODE_KIND_EXPRESSION);

	node_expr_t *expr = (node_expr_t*) node;

	assert(expr->kind == EXPRESSION_KIND_INT);

	node_expr_int_t *integer = (node_expr_int_t*) node;

	if(e_value)
		*e_value = integer->value;
}

void node_get_float(node_t *node, int64_t *e_value)
{
	assert(node->kind == NODE_KIND_EXPRESSION);

	node_expr_t *expr = (node_expr_t*) node;

	assert(expr->kind == EXPRESSION_KIND_FLOAT);

	node_expr_float_t *floating = (node_expr_float_t*) node;

	if(e_value)
		*e_value = floating->value;
}

void node_get_string(node_t *node, char **e_content, int *e_length)
{
	assert(node->kind == NODE_KIND_EXPRESSION);

	node_expr_t *expr = (node_expr_t*) node;

	assert(expr->kind == EXPRESSION_KIND_STRING);

	node_expr_string_t *string = (node_expr_string_t*) node;

	if(e_content)
		*e_content = string->content;

	if(e_length)
		*e_length = string->length;
}

int node_get_kind(node_t *node)
{
	return node->kind;
}

void node_get_location(node_t *node, int *e_offset, int *e_length)
{
	if(e_offset)
		*e_offset = node->offset;

	if(e_length)
		*e_length = node->length;
}

node_t *node_get_next(node_t *node)
{
	return node->next;
}

/* ==================== */
/* === Ast printers === */
/* ==================== */

void ast_print(ast_t ast, FILE *fp)
{
	node_print(ast.root, fp);
}

void node_print(node_t *node, FILE *fp)
{
	switch(node->kind) {

		case NODE_KIND_BREAK: 
		fprintf(fp, "break"); 
		break;
		
		case NODE_KIND_CONTINUE: 
		fprintf(fp, "continue"); 
		break;
		
		case NODE_KIND_RETURN:
		{
			fprintf(fp, "return ");
			node_print(((node_return_t*) node)->expression, fp);
		}
		break;

		case NODE_KIND_IMPORT:
		{
			node_import_t *import = (node_import_t*) node;

			fprintf(fp, "import ");

			node_print(import->expression, fp);

			if(import->name)
				fprintf(fp, " as %s", import->name);

			break;
		}

		case NODE_KIND_IFELSE:
		{
			node_ifelse_t *ifelse = (node_ifelse_t*) node;

			fprintf(fp, "if ");

			node_print(ifelse->expression, fp);

			node_print(ifelse->if_block, fp);

			if(ifelse->else_block) {

				fprintf(fp, " else ");

				node_print(ifelse->else_block, fp);
			}

			break;
		}

		case NODE_KIND_WHILE:
		{
			node_while_t *w = (node_while_t*) node;

			fprintf(fp, "while ");

			node_print(w->expression, fp);

			node_print(w->block, fp);

			break;
		}

		case NODE_KIND_ARGUMENT:
		{
			node_argument_t *argument = (node_argument_t*) node;

			fprintf(fp, "%s", argument->name);

			break;
		}

		case NODE_KIND_DICT_ITEM:
		{
			node_dict_item_t *item = (node_dict_item_t*) node;

			node_print(item->key, fp);
			fprintf(fp, ": ");
			node_print(item->value, fp);
			break;
		}
		case NODE_KIND_EXPRESSION:
		{
			node_expr_t *expr = (node_expr_t*) node;
			
			char *operation_text = 0;
			int precedes = 1;

			switch(expr->kind) {

				case EXPRESSION_KIND_INT:
				fprintf(fp, "(%ld)", ((node_expr_int_t*) node)->value);
				break;
				
				case EXPRESSION_KIND_FLOAT:
				fprintf(fp, "(%lf)", ((node_expr_float_t*) node)->value);
				break;

				case EXPRESSION_KIND_STRING:
				fprintf(fp, "(%s)", ((node_expr_string_t*) node)->content);
				break;

				case EXPRESSION_KIND_ARRAY:
				{
					node_expr_array_t *arr = (node_expr_array_t*) node;
					
					node_t *item = arr->item_head;

					fprintf(fp, "[");
					while(item) {

						node_print(item, fp);

						item = item->next;
					
						if(item)
							fprintf(fp, ", ");
					}

					fprintf(fp, "]");
					
					break;
				}

				case EXPRESSION_KIND_DICT:
				{
					node_expr_dict_t *comp = (node_expr_dict_t*) node;
					
					node_t *item = comp->item_head;

					fprintf(fp, "{ ");
					while(item) {

						node_print(item, fp);

						item = item->next;
					
						if(item)
							fprintf(fp, ", ");
					}

					fprintf(fp, " }");
					
					break;
				}

				case EXPRESSION_KIND_FUNCTION:
				{
					
					node_expr_function_t *func = (node_expr_function_t*) node;
					
					node_t *item = func->argument_head;

					fprintf(fp, "(function (");
					while(item) {

						node_print(item, fp);

						item = item->next;
					
						if(item)
							fprintf(fp, ", ");
					}

					fprintf(fp, ") ");

					node_print(func->body, fp);
					
					fprintf(fp, ")");

					break;
				
				}

				case EXPRESSION_KIND_IDENTIFIER:
				fprintf(fp, "(%s)", ((node_expr_identifier_t*) node)->content);
				break;

				case EXPRESSION_KIND_NEG: if(!operation_text) operation_text = "-"; /* FALLTHROUGH */
				case EXPRESSION_KIND_NOT:  if(!operation_text) operation_text = "!"; /* FALLTHROUGH */
				case EXPRESSION_KIND_BITWISE_NOT:  if(!operation_text) operation_text = "~"; /* FALLTHROUGH */
				case EXPRESSION_KIND_PRE_INC:  if(!operation_text) operation_text = "++"; /* FALLTHROUGH */
				case EXPRESSION_KIND_PRE_DEC:  if(!operation_text) operation_text = "--"; /* FALLTHROUGH */
				case EXPRESSION_KIND_POST_INC: if(!operation_text) { operation_text = "++"; precedes = 0; } /* FALLTHROUGH */
				case EXPRESSION_KIND_POST_DEC: if(!operation_text) { operation_text = "--"; precedes = 0; } /* FALLTHROUGH */
				{
					node_expr_operation_t *oper = (node_expr_operation_t*) node;
					if(precedes) {

						fprintf(fp, "(%s", operation_text);
						node_print(oper->operand_head, fp);
						fprintf(fp, ")");
					} else {

						fprintf(fp, "(");
						node_print(oper->operand_head, fp);
						fprintf(fp, "%s)", operation_text);
					}
				}
				break;

				case EXPRESSION_KIND_ADD:if(!operation_text) operation_text = "+"; /* FALLTHROUGH */
				case EXPRESSION_KIND_SUB:if(!operation_text) operation_text = "-"; /* FALLTHROUGH */
				case EXPRESSION_KIND_MUL:if(!operation_text) operation_text = "*"; /* FALLTHROUGH */
				case EXPRESSION_KIND_DIV:if(!operation_text) operation_text = "/"; /* FALLTHROUGH */
				case EXPRESSION_KIND_MOD:if(!operation_text) operation_text = "%"; /* FALLTHROUGH */
				case EXPRESSION_KIND_POW:if(!operation_text) operation_text = "**"; /* FALLTHROUGH */
				case EXPRESSION_KIND_LSS:if(!operation_text) operation_text = "<"; /* FALLTHROUGH */
				case EXPRESSION_KIND_GRT:if(!operation_text) operation_text = ">"; /* FALLTHROUGH */
				case EXPRESSION_KIND_LEQ:if(!operation_text) operation_text = "<="; /* FALLTHROUGH */
				case EXPRESSION_KIND_GEQ:if(!operation_text) operation_text = ">="; /* FALLTHROUGH */
				case EXPRESSION_KIND_EQL:if(!operation_text) operation_text = "=="; /* FALLTHROUGH */
				case EXPRESSION_KIND_NQL:if(!operation_text) operation_text = "!="; /* FALLTHROUGH */
				case EXPRESSION_KIND_AND:if(!operation_text) operation_text = "&&"; /* FALLTHROUGH */
				case EXPRESSION_KIND_OR:if(!operation_text) operation_text = "||"; /* FALLTHROUGH */
				case EXPRESSION_KIND_BITWISE_AND:if(!operation_text) operation_text = "&"; /* FALLTHROUGH */
				case EXPRESSION_KIND_BITWISE_OR:if(!operation_text) operation_text = "|"; /* FALLTHROUGH */
				case EXPRESSION_KIND_BITWISE_XOR:if(!operation_text) operation_text = "^"; /* FALLTHROUGH */
				case EXPRESSION_KIND_SHL:if(!operation_text) operation_text = ">>"; /* FALLTHROUGH */
				case EXPRESSION_KIND_SHR:if(!operation_text) operation_text = "<<"; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN:if(!operation_text) operation_text = "="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_ADD:if(!operation_text) operation_text = "+="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_SUB:if(!operation_text) operation_text = "-="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_MUL:if(!operation_text) operation_text = "*="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_DIV:if(!operation_text) operation_text = "/="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_MOD:if(!operation_text) operation_text = "%="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_BITWISE_AND:if(!operation_text) operation_text = "&="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_BITWISE_OR:if(!operation_text) operation_text = "|="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_BITWISE_XOR:if(!operation_text) operation_text = "^="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_SHL:if(!operation_text) operation_text = ">>="; /* FALLTHROUGH */
				case EXPRESSION_KIND_ASSIGN_SHR:if(!operation_text) operation_text = "<<="; /* FALLTHROUGH */
				case EXPRESSION_KIND_DOT_SELECTION:if(!operation_text) operation_text = "."; /* FALLTHROUGH */
				{
					node_expr_operation_t *oper = (node_expr_operation_t*) node;

					fprintf(fp, "(");
					node_print(oper->operand_head, fp);
					fprintf(fp, " %s ", operation_text);
					node_print(oper->operand_tail, fp);
					fprintf(fp, ")");
				}
				break;

				case EXPRESSION_KIND_INDEX_SELECTION:
				{
					node_expr_operation_t *oper = (node_expr_operation_t*) node;

					fprintf(fp, "(");
					node_print(oper->operand_head, fp);
					fprintf(fp, "[");
					node_print(oper->operand_tail, fp);
					fprintf(fp, "])");
				}
				break;

				case EXPRESSION_KIND_CALL:if(!operation_text) operation_text = "."; /* FALLTHROUGH */
				{
					node_expr_operation_t *oper = (node_expr_operation_t*) node;

					fprintf(fp, "(");
					node_print(oper->operand_head, fp);
					fprintf(fp, "(");

					node_t *arg = oper->operand_head->next;

					while(arg) {

						node_print(arg, fp);

						arg = arg->next;

						if(arg)
							fprintf(fp, ", ");
					}
					
					fprintf(fp, "))");
				}
				break;

				default:
				fprintf(fp, "<?>");
				break;
			}
			break;
		}

		case NODE_KIND_COMPOUND:
		{
			node_compound_t *comp = (node_compound_t*) node;
			
			node_t *stmt = comp->head;

			fprintf(fp, "{ ");
			while(stmt) {

				node_print(stmt, fp);
				fprintf(fp, "; ");

				stmt = stmt->next;
			}
			fprintf(fp, "}");
			
			break;
		}

		default:
		fprintf(fp, "<?>");
		break;
	}
}