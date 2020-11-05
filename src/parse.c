
#include <assert.h>
#include "noja.h"

node_t *parse_statement(pool_t *pool, token_iterator_t *iterator, char *source);
node_t *parse_expression(pool_t *pool, token_iterator_t *iterator, char *source);

#define FAILED fprintf(stderr, ">> Failed at %s:%d\n", __FILE__, __LINE__);

#warning "Implement parsing error reporting"

node_t *parse_function_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	token_t token = token_iterator_current(iterator);

	if(token.kind != TOKEN_KIND_KWORD_FUNCTION) {

		FAILED;

		// #ERROR
		// Unexpected token. Was expected a function expression
		return 0;
	}

	int func_expr_offset = token.offset,
		func_expr_length;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source while parsing function expression
		return 0;
	}

	token = token_iterator_current(iterator);

	if(token.kind != '(') {

		FAILED;

		// #ERROR
		// Unexpected token after function keyword. Was expected [(]
	}

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source while parsing function expression
		return 0;
	}

	token = token_iterator_current(iterator);

	node_t *argument_head = 0,
		   *argument_tail = 0;
	int 	argument_count = 0;

	if(token.kind == ')') {

		// function with no arguments!

	} else {

		// function with one or more arguments!

		token_iterator_prev(iterator);

		while(1) {

			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source while parsing function arguments
				return 0;
			}

			token = token_iterator_current(iterator);

			if(token.kind != TOKEN_KIND_IDENTIFIER) {

				FAILED;

				// #ERROR
				// Unexpected token while parsing function arguments. Was expected an identifier as argument name
				return 0;
			}

			char *name_content;

			if(!token_to_string(pool, token, source, &name_content, 0))
				return 0;

			node_t *argument = node_argument_create(pool, token.offset, token.length, name_content);

			if(argument == 0)
				return 0;

			if(!argument_head) {

				argument_head = argument;
			
			} else {

				argument_tail->next = argument;
			}

			argument_tail = argument;
			argument_count++;
		
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source while parsing function arguments
				return 0;
			}

			token = token_iterator_current(iterator);

			if(token.kind == ')')
				break;

			if(token.kind != ',') {

				FAILED;

				// #ERROR
				// Unexpected token after argument identifier. Was expected [,] or [)]
				return 0;
			}
		}
	}

	if(!token_iterator_next(iterator)) {

		// #ERROR
		// Unexpected end of source in function expression
		return 0;
	}

	// now parse the body

	node_t *body = parse_statement(pool, iterator, source);

	if(body == 0)
		return 0;

	func_expr_length = body->offset + body->length - func_expr_offset;

	return node_function_create(pool, func_expr_offset, func_expr_length, argument_head, argument_tail, argument_count, body);
}



node_t *parse_dict_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{

	int dict_offset, dict_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != '{') {

		FAILED;

		// #ERROR
		// Was expected a dict expression
		return 0;
	}

	dict_offset = token.offset;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source while parsing dict expression
		return 0;
	}

	token = token_iterator_current(iterator);

	if(token.kind == '}') {

		dict_length = token.offset + token.length - dict_offset;

		return node_dict_create(pool, dict_offset, dict_length, 0, 0, 0);
	}

	token_iterator_prev(iterator);

	node_t *item_head = 0, 
		   *item_tail = 0;
	int item_count = 0;

	while(1) {

		node_t *key, *value, *item;

		if(!token_iterator_next(iterator)) {

			FAILED;

			// #ERROR
			// Unexpected end of source while parsing dict expression
			return 0;
		}

		key = parse_expression(pool, iterator, source);

		if(key == 0)
			return 0;

		if(!token_iterator_next(iterator)) {

			FAILED;

			// #ERROR
			// Unexpected end of source while parsing dict expression
			return 0;
		}

		token = token_iterator_current(iterator);

		if(token.kind != ':') {

			FAILED;

			// #ERROR
			// Unexpected token while parsing dict expression. Was expected a [:] after key expression
			return 0;
		}

		if(!token_iterator_next(iterator)) {

			FAILED;

			// #ERROR
			// Unexpected end of source while parsing dict expression
			return 0;
		}

		value = parse_expression(pool, iterator, source);

		if(value == 0)
			return 0;

		item = node_dict_item_create(pool, key->offset, value->offset + value->length - key->offset, key, value);

		if(item == 0)
			return 0;

		if(!item_head) {

			item_head = item;

		} else {

			item_tail->next = item;
		}

		item_tail = item;
		item_count++;

		if(!token_iterator_next(iterator)) {

			FAILED;

			// #ERROR
			// Unexpected end of source while parsing dict expression
			return 0;
		}

		token = token_iterator_current(iterator);

		if(token.kind == '}')
			break;

		if(token.kind != ',') {

			FAILED;

			// #ERROR
			// Unexpected token after dict item. Was expected [,] or [}]
			return 0;
		}
	}

	dict_length = item_tail->offset + item_tail->length - dict_offset;

	return node_dict_create(pool, dict_offset, dict_length, item_head, item_tail, item_count);
}

node_t *parse_array_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{

	int array_offset, array_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != '[') {

		FAILED;

		// #ERROR
		// Was expected a array expression
		return 0;
	}

	array_offset = token.offset;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source while parsing array expression
		return 0;
	}

	token = token_iterator_current(iterator);

	if(token.kind == ']') {

		array_length = token.offset + token.length - array_offset;

		return node_array_create(pool, array_offset, array_length, 0, 0, 0);
	}

	token_iterator_prev(iterator);

	node_t *item_head = 0, 
		   *item_tail = 0;
	int item_count = 0;

	while(1) {

		node_t *item;

		if(!token_iterator_next(iterator)) {

			FAILED;

			// #ERROR
			// Unexpected end of source while parsing array expression
			return 0;
		}

		item = parse_expression(pool, iterator, source);

		if(item == 0)
			return 0;

		if(!item_head) {

			item_head = item;

		} else {

			item_tail->next = item;
		}

		item_tail = item;
		item_count++;

		if(!token_iterator_next(iterator)) {

			FAILED;

			// #ERROR
			// Unexpected end of source while parsing array expression
			return 0;
		}

		token = token_iterator_current(iterator);

		if(token.kind == ']')
			break;

		if(token.kind != ',') {

			FAILED;

			// #ERROR
			// Unexpected token after array item. Was expected [,] or []]
			return 0;
		}
	}

	array_length = item_tail->offset + item_tail->length - array_offset;

	return node_array_create(pool, array_offset, array_length, item_head, item_tail, item_count);
}

node_t *parse_subexpression_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	token_t token = token_iterator_current(iterator);

	if(token.kind != '(') {

		FAILED;

		// #ERROR
		// Was expected sub-expression
		return 0;
	}

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source inside an expression
		return 0;
	}

	node_t *node = parse_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source after expression
		return 0;
	}

	token = token_iterator_current(iterator);

	if(token.kind != ')') {

		FAILED;

		// #ERROR
		// Was expected [)] after sub-expression
		return 0;
	}

	return node;
}

node_t *parse_primary_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	token_t token = token_iterator_current(iterator);

	switch(token.kind) {

		case TOKEN_KIND_VALUE_INT: 	 
		{

			return node_int_create(pool, token.offset, token.length, token_to_int(token, source));
		}
	
		case TOKEN_KIND_VALUE_FLOAT: 
		{

			return node_float_create(pool, token.offset, token.length, token_to_float(token, source));
		}

		case TOKEN_KIND_VALUE_STRING:
		{

			char *content;
			int length;

			if(!token_to_string(pool, token, source, &content, &length))
				return 0;

			return node_string_create(pool, token.offset, token.length, content, length);
		}

		case TOKEN_KIND_IDENTIFIER:
		{

			char *content;
			int length;

			if(!token_to_string(pool, token, source, &content, &length))
				return 0;

			return node_identifier_create(pool, token.offset, token.length, content, length);
		}

		case TOKEN_KIND_KWORD_FUNCTION: 
		return parse_function_expression(pool, iterator, source);

		case '{': 
		//token_iterator_prev(iterator);
		return parse_dict_expression(pool, iterator, source);

		case '[': 
		//token_iterator_prev(iterator);
		return parse_array_expression(pool, iterator, source);

		case '(': 
		//token_iterator_prev(iterator);
		return parse_subexpression_expression(pool, iterator, source);

		default:
		FAILED;

		// #ERROR
		// Unexpected token. Was expected a pripary expression
		return 0;
	}

	assert(0);
	return 0;
}

node_t *parse_postfix_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_primary_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	char done = 0;

	while(!done) {

		if(!token_iterator_next(iterator))
			break;

		token_t token = token_iterator_current(iterator);

		#warning "Parse postfix expressions (array subscriptions, function calls and dot selections)"

		switch(token.kind) {
			
			case '[': 
			{
				if(!token_iterator_next(iterator)) {

					FAILED;
					
					// #ERROR
					// Unexpected end of source while parsing index selection
					return 0;
				}

				node_t *index = parse_expression(pool, iterator, source);

				if(index == 0)
					return 0;

				if(!token_iterator_next(iterator)) {

					FAILED;
					
					// #ERROR
					// Unexpected end of source after index selection expression. Was expecred []]
					return 0;
				}

				token = token_iterator_current(iterator);

				if(token.kind != ']') {

					FAILED;

					// #ERROR
					// Unexpected token. Was expected []] after index selection expression
					return 0;
				}

				node = node_index_selection_create(pool, node->offset, token.offset + token.length - node->offset, node, index);
				break;
			}

			case '(': assert(0); break;
			case '.': assert(0); break;
			case TOKEN_KIND_OPERATOR_INC: node = node_post_inc_create(pool, node->offset, token.offset + token.length - node->offset, node); break;
			case TOKEN_KIND_OPERATOR_DEC: node = node_post_dec_create(pool, node->offset, token.offset + token.length - node->offset, node); break;
			default:
			token_iterator_prev(iterator);
			done = 1;
		}

		if(node == 0)
			return 0;
	}

	return node;
}

node_t *parse_unary_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	token_t token = token_iterator_current(iterator);

	switch(token.kind) {

		case TOKEN_KIND_OPERATOR_ADD:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source in expression
				return 0;
			}

			node_t *node = parse_unary_expression(pool, iterator, source);

			if(node == 0)
				return 0;

			return node;
		}

		case TOKEN_KIND_OPERATOR_SUB:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source in expression
				return 0;
			}

			node_t *node = parse_unary_expression(pool, iterator, source);

			if(node == 0)
				return 0;

			return node_neg_create(pool, token.offset, node->offset + node->length - token.offset, node);
		}

		case TOKEN_KIND_OPERATOR_NOT:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source in expression
				return 0;
			}

			node_t *node = parse_unary_expression(pool, iterator, source);

			if(node == 0)
				return 0;

			return node_not_create(pool, token.offset, node->offset + node->length - token.offset, node);
		}

		case TOKEN_KIND_OPERATOR_BITWISE_NOT:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source in expression
				return 0;
			}

			node_t *node = parse_unary_expression(pool, iterator, source);

			if(node == 0)
				return 0;

			return node_bitwise_not_create(pool, token.offset, node->offset + node->length - token.offset, node);
		}
	}

	return parse_postfix_expression(pool, iterator, source);
}

node_t *parse_multiplicative_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_unary_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_MUL:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after multiplicative operator
				return 0;
			}

			node_t *right = parse_multiplicative_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_mul_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_DIV:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after multiplicative operator
				return 0;
			}

			node_t *right = parse_multiplicative_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_div_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_MOD:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after multiplicative operator
				return 0;
			}

			node_t *right = parse_multiplicative_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_mod_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_additive_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_multiplicative_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_ADD:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after additive operator
				return 0;
			}

			node_t *right = parse_additive_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_add_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_SUB:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after additive operator
				return 0;
			}

			node_t *right = parse_additive_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_sub_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_shift_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_additive_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_SHL:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after shift operator
				return 0;
			}

			node_t *right = parse_shift_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_shl_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_SHR:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after shift operator
				return 0;
			}

			node_t *right = parse_shift_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_shr_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_relational_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_shift_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_LSS:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after relational operator
				return 0;
			}

			node_t *right = parse_relational_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_lss_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_GRT:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after relational operator
				return 0;
			}

			node_t *right = parse_relational_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_grt_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_LEQ:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after relational operator
				return 0;
			}

			node_t *right = parse_relational_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_leq_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_GEQ:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after relational operator
				return 0;
			}

			node_t *right = parse_relational_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_geq_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_equality_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_relational_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_EQL:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after equality operator
				return 0;
			}

			node_t *right = parse_equality_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_eql_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_NQL:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after equality operator
				return 0;
			}

			node_t *right = parse_equality_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_nql_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_bitwise_and_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_equality_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_BITWISE_AND:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after bitwise and operator
				return 0;
			}

			node_t *right = parse_bitwise_and_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_bitwise_and_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_bitwise_xor_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_bitwise_and_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_BITWISE_XOR:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after bitwise xor operator
				return 0;
			}

			node_t *right = parse_bitwise_xor_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_bitwise_xor_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_bitwise_or_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_bitwise_xor_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_BITWISE_OR:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after bitwise or operator
				return 0;
			}

			node_t *right = parse_bitwise_or_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_bitwise_or_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_and_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_bitwise_or_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_AND:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after and operator
				return 0;
			}

			node_t *right = parse_and_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_and_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_or_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_and_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_AND:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after or operator
				return 0;
			}

			node_t *right = parse_or_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_or_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_assign_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	node_t *node = parse_or_expression(pool, iterator, source);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_ASSIGN:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_ADD:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_add_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_SUB:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_sub_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
		case TOKEN_KIND_OPERATOR_ASSIGN_MUL:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_mul_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_DIV:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_div_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_MOD:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_mod_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_AND:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_bitwise_and_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_OR:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_bitwise_or_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_XOR:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_bitwise_xor_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_SHL:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_shl_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

		case TOKEN_KIND_OPERATOR_ASSIGN_SHR:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after assignment operator
				return 0;
			}

			node_t *right = parse_assign_expression(pool, iterator, source);

			if(right == 0)
				return 0;

			return node_assign_shr_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}


node_t *parse_expression(pool_t *pool, token_iterator_t *iterator, char *source)
{
	return parse_assign_expression(pool, iterator, source);
}

node_t *parse_while_statement(pool_t *pool, token_iterator_t *iterator, char *source)
{
	int while_stmt_offset,
		while_stmt_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != TOKEN_KIND_KWORD_WHILE) {

		FAILED;

		// #ERROR
		// Was expected a while statement
		return 0;
	}

	while_stmt_offset = token.offset;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source after while keyword. Was expected an expression
		return 0;
	}

	node_t *expression = parse_expression(pool, iterator, source);

	if(expression == 0)
		return 0;

	if(!token_iterator_next(iterator)) {

		// #ERROR
		// Unexpected end of source in while statement, after the expression
		FAILED;

		return 0;
	}

	node_t *block = parse_statement(pool, iterator, source);

	if(block == 0)
		return 0;

	while_stmt_length = block->offset + block->length - while_stmt_offset;

	return node_while_create(pool, while_stmt_offset, while_stmt_length, expression, block);
}

node_t *parse_ifelse_statement(pool_t *pool, token_iterator_t *iterator, char *source)
{
	int ifelse_stmt_offset,
		ifelse_stmt_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != TOKEN_KIND_KWORD_IF) {

		FAILED;

		// #ERROR
		// Was expected an if-else statement
		return 0;
	}

	ifelse_stmt_offset = token.offset;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source after if keyword. Was expected an expression
		return 0;
	}

	node_t *expression = parse_expression(pool, iterator, source);

	if(expression == 0)
		return 0;

	if(!token_iterator_next(iterator)) {

		// #ERROR
		// Unexpected end of source in if statement, after the expression
		FAILED;

		return 0;
	}

	node_t *if_block = parse_statement(pool, iterator, source);

	if(if_block == 0)
		return 0;

	node_t *else_block = 0;
	ifelse_stmt_length = if_block->offset + if_block->length - ifelse_stmt_offset;

	if(token_iterator_next(iterator)) {

		token = token_iterator_current(iterator);

		if(token.kind == TOKEN_KIND_KWORD_ELSE) {

			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after else keyword
				return 0;
			}

			else_block = parse_statement(pool, iterator, source);

			ifelse_stmt_length = else_block->offset + else_block->length - ifelse_stmt_offset;

		} else {

			token_iterator_prev(iterator);
		}
	}

	return node_ifelse_create(pool, ifelse_stmt_offset, ifelse_stmt_length, expression, if_block, else_block);
}

node_t *parse_statement(pool_t *pool, token_iterator_t *iterator, char *source)
{
	token_t token = token_iterator_current(iterator);

	switch(token.kind) {

		case '{':
		{
			if(!token_iterator_next(iterator)) {

				// #ERROR
				// Unexpected end of source in compound statement
				return 0;
			}

			int comp_stmt_offset,
				comp_stmt_length;

			comp_stmt_offset = token.offset;

			node_t *stmt_head = 0, 
		   		   *stmt_tail = 0;
		   	int stmt_count = 0;

			while(1) {

				node_t *stmt = parse_statement(pool, iterator, source);

				if(stmt == 0)
					return 0;

				if(!stmt_head) {

					stmt_head = stmt;
					
				} else {

					stmt_tail->next = stmt;
				}

				stmt_tail = stmt;
				stmt_count++;

				if(!token_iterator_next(iterator)) {

					// #ERROR
					// Unexpected and of source inside compound statement
					break;
				}

				token = token_iterator_current(iterator);

				if(token.kind == '}')
					break;
			}

			comp_stmt_length = token.offset + token.length - comp_stmt_offset;

			return node_compound_create(pool, comp_stmt_offset, comp_stmt_length, stmt_head, stmt_tail, stmt_count);
		}

		case '(':
		case '[':
		case TOKEN_KIND_VALUE_INT:
		case TOKEN_KIND_VALUE_FLOAT:
		case TOKEN_KIND_VALUE_STRING:
		case TOKEN_KIND_IDENTIFIER:
		case TOKEN_KIND_KWORD_FUNCTION:
		case TOKEN_KIND_OPERATOR_ADD:
		case TOKEN_KIND_OPERATOR_SUB:
		case TOKEN_KIND_OPERATOR_INC:
		case TOKEN_KIND_OPERATOR_DEC:
		case TOKEN_KIND_OPERATOR_NOT:
		case TOKEN_KIND_OPERATOR_BITWISE_NOT:
		{
			node_t *expression = parse_expression(pool, iterator, source);
			
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after expression. Was expected [;]
				return 0;
			}
			
			token = token_iterator_current(iterator);

			if(token.kind != ';') {

				FAILED;

				// #ERROR
				// Unexpected token after expression. Was expected [;]

				return 0;
			}

			return expression;
		}

		case TOKEN_KIND_KWORD_BREAK:
		return node_break_create(pool, token.offset, token.length);

		case TOKEN_KIND_KWORD_CONTINUE:
		return node_continue_create(pool, token.offset, token.length);

		case TOKEN_KIND_KWORD_ELSE:
		{
			FAILED;

			// #ERROR
			// Unexpected else keyword. An else statement should follow an if statement
			return 0;
		}

		case TOKEN_KIND_KWORD_IF:
		return parse_ifelse_statement(pool, iterator, source);

		case TOKEN_KIND_KWORD_WHILE:
		return parse_while_statement(pool, iterator, source);

		case TOKEN_KIND_KWORD_IMPORT:
		#warning "Parse import statements"
		assert(0); // Unsupported!

		case TOKEN_KIND_KWORD_RETURN:
		{
			int return_stmt_offset,
				return_stmt_length;

			return_stmt_offset = token.offset;

			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after return keyword. Was expected an expression
				return 0;
			}

			node_t *expression = parse_expression(pool, iterator, source);
			
			if(expression == 0)
				return 0;

			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after expression. Was expected [;]
				return 0;
			}

			token = token_iterator_current(iterator);

			if(token.kind != ';') {

				FAILED;

				// #ERROR
				// Unexpected token after return statement. Was expected [;]
				return 0;
			}

			return_stmt_length = token.offset + token.length - return_stmt_offset;

			return node_return_create(pool, return_stmt_offset, return_stmt_length, expression);
		}

		default:
		FAILED;

		// #ERROR
		// Unexpected token at the start of statement
		assert(0);
	}
}

int parse(token_array_t *array, char *source, pool_t **e_pool, node_t **e_node)
{
	token_iterator_t iterator;
	token_iterator_init(&iterator, array);

	pool_t *pool = pool_create();

	if(pool == 0)
		return 0;

	node_t *stmt_head = 0, 
		   *stmt_tail = 0;
	int 	stmt_count = 0;

	while(1) {

		node_t *stmt = parse_statement(pool, &iterator, source);

		if(stmt == 0) {
			
			pool_destroy(pool);
			return 0;
		}

		{
			if(!stmt_head) {

				stmt_head = stmt;
			
			} else {

				stmt_tail->next = stmt;
			}

			stmt_tail = stmt;
			stmt_count++;
		}

		if(!token_iterator_next(&iterator))

			// The source ended
			break;
	}

	node_t *result = node_compound_create(pool, stmt_head->offset, stmt_tail->offset + stmt_tail->length - stmt_head->offset, stmt_head, stmt_tail, stmt_count);

	if(result == 0) {

		pool_destroy(pool);
		return 0;
	}

	*e_pool = pool;
	*e_node = result;
	return 1;
}