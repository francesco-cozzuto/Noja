
#include <stdint.h>
#include <stdio.h>
#include "utils/pool.h"

enum {
	NODE_KIND_BREAK,
	NODE_KIND_CONTINUE,
	NODE_KIND_RETURN,
	NODE_KIND_IFELSE,
	NODE_KIND_WHILE,
	NODE_KIND_ARGUMENT,
	NODE_KIND_DICT_ITEM,
	NODE_KIND_EXPRESSION,
	NODE_KIND_COMPOUND,
};

enum {
	EXPRESSION_KIND_INT,
	EXPRESSION_KIND_FLOAT,
	EXPRESSION_KIND_STRING,
	EXPRESSION_KIND_ARRAY,
	EXPRESSION_KIND_DICT,
	EXPRESSION_KIND_FUNCTION,
	EXPRESSION_KIND_IDENTIFIER,

	EXPRESSION_KIND_NEG,
	EXPRESSION_KIND_ADD,
	EXPRESSION_KIND_SUB,
	EXPRESSION_KIND_MUL,
	EXPRESSION_KIND_DIV,
	EXPRESSION_KIND_MOD,
	EXPRESSION_KIND_POW,
	EXPRESSION_KIND_LSS,
	EXPRESSION_KIND_GRT,
	EXPRESSION_KIND_LEQ,
	EXPRESSION_KIND_GEQ,
	EXPRESSION_KIND_EQL,
	EXPRESSION_KIND_NQL,
	EXPRESSION_KIND_AND,
	EXPRESSION_KIND_OR,
	EXPRESSION_KIND_NOT,

	EXPRESSION_KIND_BITWISE_AND,
	EXPRESSION_KIND_BITWISE_OR,
	EXPRESSION_KIND_BITWISE_XOR,
	EXPRESSION_KIND_BITWISE_NOT,

	EXPRESSION_KIND_SHL,
	EXPRESSION_KIND_SHR,

	EXPRESSION_KIND_PRE_INC,
	EXPRESSION_KIND_PRE_DEC,
	EXPRESSION_KIND_POST_INC,
	EXPRESSION_KIND_POST_DEC,

	EXPRESSION_KIND_ASSIGN,
	EXPRESSION_KIND_ASSIGN_ADD,
	EXPRESSION_KIND_ASSIGN_SUB,
	EXPRESSION_KIND_ASSIGN_MUL,
	EXPRESSION_KIND_ASSIGN_DIV,
	EXPRESSION_KIND_ASSIGN_MOD,
	EXPRESSION_KIND_ASSIGN_BITWISE_AND,
	EXPRESSION_KIND_ASSIGN_BITWISE_OR,
	EXPRESSION_KIND_ASSIGN_BITWISE_XOR,
	EXPRESSION_KIND_ASSIGN_SHL,
	EXPRESSION_KIND_ASSIGN_SHR,

	EXPRESSION_KIND_INDEX_SELECTION,
	EXPRESSION_KIND_DOT_SELECTION,
	EXPRESSION_KIND_CALL,
};

typedef struct node_t node_t;
struct node_t {
	int kind;
	int offset;
	int length;
	node_t *next;
};

typedef struct ast_t ast_t;
struct ast_t {
	node_t *root;
	pool_t *pool;
};

typedef struct {
	node_t super;
	int kind;
} node_expr_t;

typedef struct {
	node_expr_t super;
	int operand_count;
	node_t *operand_head, *operand_tail;
} node_expr_operation_t;

typedef struct {
	node_expr_t super;
	char *content;
	int length;
} node_expr_string_t;

typedef struct {
	node_expr_t super;
	int64_t value;
} node_expr_int_t;

typedef struct {
	node_expr_t super;
	double value;
} node_expr_float_t;

typedef struct {
	node_expr_t super;
	char *content;
	int length;
} node_expr_identifier_t;

typedef struct {
	node_t super;
	char *name;
} node_argument_t;

typedef struct {
	node_expr_t super;
	node_t *argument_head, *argument_tail;
	int argument_count;
	node_t *body;
} node_expr_function_t;

typedef struct {
	node_expr_t super;
	node_t *item_head, *item_tail;
	int item_count;
} node_expr_array_t;

typedef struct {
	node_t super;
	node_t *key, *value;
} node_dict_item_t;

typedef struct {
	node_expr_t super;
	node_t *item_head, *item_tail;
	int item_count;
} node_expr_dict_t;

typedef struct {
	node_t super;
	node_t *expression;
	node_t *block;
} node_while_t;

typedef struct {
	node_t super;
	node_t *expression;
	node_t *if_block;
	node_t *else_block;
} node_ifelse_t;

typedef struct {
	node_t super;
	node_t *expression;
} node_return_t;

typedef struct {
	node_t super;
	node_t *head, *tail;
	int count;
} node_compound_t;

void ast_delete(ast_t ast);
node_t *node_neg_create(pool_t *pool, int offset, int length, node_t *operand);
node_t *node_add_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_sub_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_mul_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_div_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_mod_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_pow_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_lss_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_grt_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_leq_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_geq_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_eql_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_nql_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_and_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_or_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_not_create(pool_t *pool, int offset, int length, node_t *operand);
node_t *node_bitwise_and_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_bitwise_or_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_bitwise_xor_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_bitwise_not_create(pool_t *pool, int offset, int length, node_t *operand);
node_t *node_shl_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_shr_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_post_inc_create(pool_t *pool, int offset, int length, node_t *operand);
node_t *node_post_dec_create(pool_t *pool, int offset, int length, node_t *operand);
node_t *node_pre_inc_create(pool_t *pool, int offset, int length, node_t *operand);
node_t *node_pre_dec_create(pool_t *pool, int offset, int length, node_t *operand);
node_t *node_assign_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_add_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_sub_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_mul_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_div_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_mod_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_bitwise_and_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_bitwise_or_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_bitwise_xor_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_shl_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_assign_shr_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_dot_selection_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_index_selection_create(pool_t *pool, int offset, int length, node_t *left_operand, node_t *right_operand);
node_t *node_call_create(pool_t *pool, int offset, int length, node_t *arg_head, node_t *arg_tail, int argc);
node_t *node_int_create(pool_t *pool, int offset, int length, int64_t value);
node_t *node_float_create(pool_t *pool, int offset, int length, double value);
node_t *node_string_create(pool_t *pool, int offset, int length, char *content, int content_length);
node_t *node_identifier_create(pool_t *pool, int offset, int length, char *content, int content_length);
node_t *node_argument_create(pool_t *pool, int offset, int length, char *name);
node_t *node_function_create(pool_t *pool, int offset, int length, node_t *argument_head, node_t *argument_tail, int argument_count, node_t *body);
node_t *node_dict_item_create(pool_t *pool, int offset, int length, node_t *key, node_t *value);
node_t *node_dict_create(pool_t *pool, int offset, int length, node_t *item_head, node_t *item_tail, int item_count);
node_t *node_array_create(pool_t *pool, int offset, int length, node_t *item_head, node_t *item_tail, int item_count);
node_t *node_while_create(pool_t *pool, int offset, int length, node_t *expression, node_t *block);
node_t *node_ifelse_create(pool_t *pool, int offset, int length, node_t *expression, node_t *if_block, node_t *else_block);
node_t *node_return_create(pool_t *pool, int offset, int length, node_t *expression);
node_t *node_break_create(pool_t *pool, int offset, int length);
node_t *node_continue_create(pool_t *pool, int offset, int length);
node_t *node_compound_create(pool_t *pool, int offset, int length, node_t *head, node_t *tail, int count);

int 	node_get_kind(node_t *node);
node_t *node_get_next(node_t *node);
void 	node_get_location(node_t *node, int *e_offset, int *e_length);
void 	node_get_int(node_t *node, int64_t *e_value);
void 	node_get_float(node_t *node, int64_t *e_value);
void 	node_get_string(node_t *node, char **e_content, int *e_length);
void 	node_print(node_t *node, FILE *fp);