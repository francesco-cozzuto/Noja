
#include <assert.h>
#include "token.h"
#include "ast.h"

void find_line_range(const char *source, int source_length, int offset, int *prev_line_offset, int *line_offset, int *next_line_offset, int *line_no)
{
	int i = 0, curr_line_no = 1;

	*line_no = 1;
	*prev_line_offset = -1;
	*line_offset = -1;
	*next_line_offset = -1;

	while(i < offset) {

		if(source[i] == '\n' && i + 1 < source_length) {

			*prev_line_offset = *line_offset;
			*line_offset = i + 1;

			curr_line_no++;
		}

		i++;
	}

	// now serch for the offset of the next line

	while(i < source_length) {

		if(source[i] == '\n' && i + 1 < source_length) {
			*next_line_offset = i + 1;
			break;
		}

		i++;
	}

	*line_no = curr_line_no;
}

void print_line(string_builder_t *output_builder, const char *source, int source_length, int offset)
{
	int i = 0;
	char c;

	while(1) {

		if(offset + i == source_length)
			break;

		c = source[offset + i];

		if(c == '\n')
			break;

		string_builder_append_byte(output_builder, c);

		i++;
	}
}

static void print_unexpected_token_location(string_builder_t *output_builder, const char *source, int source_length, token_t token)
{
	int prev_line_offset,
		next_line_offset,
		line_offset,
		line_no;

	find_line_range(source, source_length, token.offset, &prev_line_offset, &line_offset, &next_line_offset, &line_no);

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

static void print_missing_semicolon_location(string_builder_t *output_builder, const char *source, int source_length, token_t token)
{
	int prev_line_offset,
		next_line_offset,
		line_offset,
		line_no;

	find_line_range(source, source_length, token.offset, &prev_line_offset, &line_offset, &next_line_offset, &line_no);

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

int tokenize(const char *source, int source_length, token_array_t *e_token_array);
int check(node_t *node, const char *source, int source_length, string_builder_t *output_builder);

node_t *parse_statement(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder);
node_t *parse_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder);

#define FAILED fprintf(stderr, ">> Failed at %s:%d\n", __FILE__, __LINE__);

#warning "Implement parsing error reporting"

node_t *parse_function_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	token_t token = token_iterator_current(iterator);

	if(token.kind != TOKEN_KIND_KWORD_FUNCTION) {

		FAILED;

		// #ERROR
		// Unexpected token. Was expected a function expression

		string_builder_append(output_builder, "Unexpected token [${string-with-length}]. Was expected a function expression", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
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

		string_builder_append(output_builder, "Unexpected token [${string-with-length}] in function expression, after the function keyword. Was expected an argument list", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
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

				string_builder_append(output_builder, "Unexpected token [${string-with-length}] inside function argument list. Was expected an identifier", source + token.offset, token.length);
				print_unexpected_token_location(output_builder, source, source_length, token);
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

				string_builder_append(output_builder, "Unexpected token [${string-with-length}] inside function argument list. Was expected [,] or [)]", source + token.offset, token.length);
				print_unexpected_token_location(output_builder, source, source_length, token);
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

	node_t *body = parse_statement(pool, iterator, source, source_length, output_builder);

	if(body == 0)
		return 0;

	func_expr_length = body->offset + body->length - func_expr_offset;

	return node_function_create(pool, func_expr_offset, func_expr_length, argument_head, argument_tail, argument_count, body);
}



node_t *parse_dict_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{

	int dict_offset, dict_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != '{') {

		FAILED;

		// #ERROR
		// Was expected a dict expression

		string_builder_append(output_builder, "Unexpected token [${string-with-length}]. Was expected a dict expression", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
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

		key = parse_expression(pool, iterator, source, source_length, output_builder);

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

			string_builder_append(output_builder, "Unexpected token [${string-with-length}] after dict item key. Was expected [:]", source + token.offset, token.length);
			print_unexpected_token_location(output_builder, source, source_length, token);
			return 0;
		}

		if(!token_iterator_next(iterator)) {

			FAILED;

			// #ERROR
			// Unexpected end of source while parsing dict expression
			return 0;
		}

		value = parse_expression(pool, iterator, source, source_length, output_builder);

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

			string_builder_append(output_builder, "Unexpected token [${string-with-length}] inside a dict expression. Was expected [,] or [}]", source + token.offset, token.length);
			print_unexpected_token_location(output_builder, source, source_length, token);
			return 0;
		}
	}

	dict_length = item_tail->offset + item_tail->length - dict_offset;

	return node_dict_create(pool, dict_offset, dict_length, item_head, item_tail, item_count);
}

node_t *parse_array_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{

	int array_offset, array_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != '[') {

		FAILED;

		// #ERROR
		// Was expected a array expression

		string_builder_append(output_builder, "Unexpected token [${string-with-length}]. Was expected an array expression", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
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

		item = parse_expression(pool, iterator, source, source_length, output_builder);

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

			string_builder_append(output_builder, "Unexpected token [${string-with-length}] inside an array expression. Was expected [,] or []]", source + token.offset, token.length);
			print_unexpected_token_location(output_builder, source, source_length, token);
			return 0;
		}
	}

	array_length = item_tail->offset + item_tail->length - array_offset;

	return node_array_create(pool, array_offset, array_length, item_head, item_tail, item_count);
}

node_t *parse_subexpression_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	token_t token = token_iterator_current(iterator);

	if(token.kind != '(') {

		FAILED;

		// #ERROR
		// Was expected sub-expression

		string_builder_append(output_builder, "Unexpected token [${string-with-length}]. Was expected a sub-expression!", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
		return 0;
	}

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source inside an expression
		return 0;
	}

	node_t *node = parse_expression(pool, iterator, source, source_length, output_builder);

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

		string_builder_append(output_builder, "Unexpected token [${string-with-length}] at the end of a sub-expression. Was expected [)]", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
		return 0;
	}

	return node;
}

node_t *parse_primary_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	token_t token = token_iterator_current(iterator);

	switch(token.kind) {

		case TOKEN_KIND_KWORD_NULL:
		return node_null_create(pool, token.offset, token.length);

		case TOKEN_KIND_KWORD_TRUE:
		return node_true_create(pool, token.offset, token.length);

		case TOKEN_KIND_KWORD_FALSE:
		return node_false_create(pool, token.offset, token.length);

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
		return parse_function_expression(pool, iterator, source, source_length, output_builder);

		case '{': 
		//token_iterator_prev(iterator);
		return parse_dict_expression(pool, iterator, source, source_length, output_builder);

		case '[': 
		//token_iterator_prev(iterator);
		return parse_array_expression(pool, iterator, source, source_length, output_builder);

		case '(': 
		//token_iterator_prev(iterator);
		return parse_subexpression_expression(pool, iterator, source, source_length, output_builder);

		case '\'':
		FAILED;

		// #ERROR
		// Unexpected token. Was expected a primary expression
		string_builder_append(output_builder, "Unexpected token [${string-with-length}] at the start of primary expression. Was expected a constant value (an int, float or string), a sub-expression, an identifier, a dict expression or an array expression. This language only used double quotes!", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
		return 0;
		break;

		default:
		FAILED;

		// #ERROR
		// Unexpected token. Was expected a primary expression
		
		string_builder_append(output_builder, "Unexpected token [${string-with-length}] at the start of primary expression. Was expected a constant value (an int, float or string), a sub-expression, an identifier, a dict expression or an array expression", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
		return 0;
	}

	assert(0);
	return 0;
}

node_t *parse_postfix_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_primary_expression(pool, iterator, source, source_length, output_builder);

	if(node == 0)
		return 0;

	char done = 0;

	while(!done) {

		if(!token_iterator_next(iterator))
			break;

		token_t token = token_iterator_current(iterator);

		switch(token.kind) {
			
			case '[': 
			{
				if(!token_iterator_next(iterator)) {

					FAILED;
					
					// #ERROR
					// Unexpected end of source while parsing index selection
					return 0;
				}

				node_t *index = parse_expression(pool, iterator, source, source_length, output_builder);

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
					string_builder_append(output_builder, "Unexpected token [${string-with-length}] after index expression in index selection. Was expected []]!", source + token.offset, token.length);
					print_unexpected_token_location(output_builder, source, source_length, token);
					return 0;
				}

				node = node_index_selection_create(pool, node->offset, token.offset + token.length - node->offset, node, index);
				break;
			}

			case '(': 
			{
				// Check if the call is without arguments

				if(!token_iterator_next(iterator)) {

					FAILED;
					
					// #ERROR
					// Unexpected end of source while parsing call expression
					return 0;
				}

				token = token_iterator_current(iterator);

				if(token.kind == ')') {

					// Call with no arguments
					node = node_call_create(pool, node->offset, token.offset + token.length - node->offset, node, node, 0);
					break;
				}

				token_iterator_prev(iterator);

				node_t *arg_head, *arg_tail;
				int argc = 0;

				arg_head = node;
				arg_tail = node;

				while(1) {

					if(!token_iterator_next(iterator)) {

						FAILED;
					
						// #ERROR
						// Unexpected end of source while parsing call expression
						return 0;
					}

					node_t *arg = parse_expression(pool, iterator, source, source_length, output_builder);

					if(arg == 0)
						return 0;

					arg_tail->next = arg;
					arg_tail = arg;
					argc++;

					if(!token_iterator_next(iterator)) {

						FAILED;
					
						// #ERROR
						// Unexpected end of source while parsing call expression
						return 0;
					}

					token = token_iterator_current(iterator);

					if(token.kind == ')')
						break;

					if(token.kind != ',') {

						FAILED;

						// #ERROR
						// Unexpected token in call expression. Was expected [,] or [)]

						string_builder_append(output_builder, "Unexpected token [${string-with-length}] in call expression. Were expected [,] or [)]!", source + token.offset, token.length);
						print_unexpected_token_location(output_builder, source, source_length, token);
						return 0;
					}
				}

				node = node_call_create(pool, node->offset, token.offset + token.length - node->offset, arg_head, arg_tail, argc);
				break;
			}
			
			case '.':
			{
				if(!token_iterator_next(iterator)) {

					FAILED;
					
					// #ERROR
					// Unexpected end of source while parsing dot selection
					return 0;
				}

				token = token_iterator_current(iterator);

				if(token.kind != TOKEN_KIND_IDENTIFIER) {

					FAILED;

					// #ERROR
					// Unexpected token after dot in dot selection. Was expected an identifier
					string_builder_append(output_builder, "Unexpected token [${string-with-length}] after dot in dot selection. Was expected an identifier!", source + token.offset, token.length);
					print_unexpected_token_location(output_builder, source, source_length, token);
					return 0;
				}

				char *content;
				int length;

				if(!token_to_string(pool, token, source, &content, &length))
					return 0;

				node_t *iden = node_identifier_create(pool, token.offset, token.length, content, length);

				if(iden == 0)
					return 0;

				node = node_dot_selection_create(pool, node->offset, iden->offset + iden->length - node->offset, node, iden);
				break;
			}

			case TOKEN_KIND_OPERATOR_ARW:
			{
				if(!token_iterator_next(iterator)) {

					FAILED;
					
					// #ERROR
					// Unexpected end of source while parsing dot selection
					return 0;
				}

				token = token_iterator_current(iterator);

				if(token.kind != TOKEN_KIND_IDENTIFIER) {

					FAILED;

					// #ERROR
					// Unexpected token after dot in dot selection. Was expected an identifier
					string_builder_append(output_builder, "Unexpected token [${string-with-length}] after arrow in arrow selection. Was expected an identifier!", source + token.offset, token.length);
					print_unexpected_token_location(output_builder, source, source_length, token);
					return 0;
				}

				char *content;
				int length;

				if(!token_to_string(pool, token, source, &content, &length))
					return 0;

				node_t *index = node_string_create(pool, token.offset, token.length, content, length);

				if(index == 0)
					return 0;

				node =  node_index_selection_create(pool, node->offset, token.offset + token.length - node->offset, node, index);
				break;
			}

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

node_t *parse_unary_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
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

			node_t *node = parse_unary_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *node = parse_unary_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *node = parse_unary_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *node = parse_unary_expression(pool, iterator, source, source_length, output_builder);

			if(node == 0)
				return 0;

			return node_bitwise_not_create(pool, token.offset, node->offset + node->length - token.offset, node);
		}
	}

	return parse_postfix_expression(pool, iterator, source, source_length, output_builder);
}

node_t *parse_multiplicative_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_unary_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_multiplicative_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_multiplicative_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_multiplicative_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_mod_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}

	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_additive_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_multiplicative_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_additive_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_additive_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_sub_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_shift_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_additive_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_shift_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_shift_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_shr_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_relational_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_shift_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_relational_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_relational_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_relational_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_relational_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_geq_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_equality_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_relational_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_equality_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_equality_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_nql_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_bitwise_and_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_equality_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_bitwise_and_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_bitwise_and_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_bitwise_xor_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_bitwise_and_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_bitwise_xor_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_bitwise_xor_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_bitwise_or_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_bitwise_xor_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_bitwise_or_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_bitwise_or_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_and_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_bitwise_or_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_and_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_and_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_or_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_and_expression(pool, iterator, source, source_length, output_builder);

	if(node == 0)
		return 0;

	if(!token_iterator_next(iterator))
		return node;

	token_t token = token_iterator_current(iterator);

	switch(token.kind) {
		case TOKEN_KIND_OPERATOR_OR:
		{
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after or operator
				return 0;
			}

			node_t *right = parse_or_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_or_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}

node_t *parse_assign_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	node_t *node = parse_or_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

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

			node_t *right = parse_assign_expression(pool, iterator, source, source_length, output_builder);

			if(right == 0)
				return 0;

			return node_assign_shr_create(pool, node->offset, right->offset + right->length - node->offset, node, right);
		}
	}

	token_iterator_prev(iterator);

	return node;
}


node_t *parse_expression(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	return parse_assign_expression(pool, iterator, source, source_length, output_builder);
}

node_t *parse_while_statement(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	int while_stmt_offset,
		while_stmt_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != TOKEN_KIND_KWORD_WHILE) {

		FAILED;

		// #ERROR
		// Was expected a while statement
		string_builder_append(output_builder, "Unexpected token [${string-with-length}]. Was expected a while statement", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
		return 0;
	}

	while_stmt_offset = token.offset;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source after while keyword. Was expected an expression

		string_builder_append(output_builder, "Unexpected end of source inside a while statement, right after the while keyword. Was expected the condition expression");
		return 0;
	}

	node_t *expression = parse_expression(pool, iterator, source, source_length, output_builder);

	if(expression == 0)
		return 0;

	if(!token_iterator_next(iterator)) {

		// #ERROR
		// Unexpected end of source in while statement, after the expression

		string_builder_append(output_builder, "Unexpected end of source inside a while statement, after the condition expression. Was expected a statement");

		FAILED;

		return 0;
	}

	node_t *block = parse_statement(pool, iterator, source, source_length, output_builder);

	if(block == 0)
		return 0;

	while_stmt_length = block->offset + block->length - while_stmt_offset;

	return node_while_create(pool, while_stmt_offset, while_stmt_length, expression, block);
}

node_t *parse_ifelse_statement(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	int ifelse_stmt_offset,
		ifelse_stmt_length;

	token_t token = token_iterator_current(iterator);

	if(token.kind != TOKEN_KIND_KWORD_IF) {

		FAILED;

		// #ERROR
		// Was expected an if-else statement

		string_builder_append(output_builder, "Unexpected token [${string-with-length}]. Was expected an if-else statement", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
		return 0;
	}

	ifelse_stmt_offset = token.offset;

	if(!token_iterator_next(iterator)) {

		FAILED;

		// #ERROR
		// Unexpected end of source after if keyword. Was expected an expression

		string_builder_append(output_builder, "Unexpected end of source inside an if-else statement, right after the if keyword. Was expected the condition expression");
		return 0;
	}

	node_t *expression = parse_expression(pool, iterator, source, source_length, output_builder);

	if(expression == 0)
		return 0;

	if(!token_iterator_next(iterator)) {

		// #ERROR
		// Unexpected end of source in if statement, after the expression

		string_builder_append(output_builder, "Unexpected end of source inside of an if-else statement, after the condition expression. Was expected a statement");

		FAILED;

		return 0;
	}

	node_t *if_block = parse_statement(pool, iterator, source, source_length, output_builder);

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
				string_builder_append(output_builder, "Unexpected end of source inside of an if-else statement, after the else keyword. Was expected a statement");
				return 0;
			}

			else_block = parse_statement(pool, iterator, source, source_length, output_builder);

			ifelse_stmt_length = else_block->offset + else_block->length - ifelse_stmt_offset;

		} else {

			token_iterator_prev(iterator);
		}
	}

	return node_ifelse_create(pool, ifelse_stmt_offset, ifelse_stmt_length, expression, if_block, else_block);
}

node_t *parse_statement(pool_t *pool, token_iterator_t *iterator, const char *source, int source_length, string_builder_t *output_builder)
{
	token_t token = token_iterator_current(iterator);

	switch(token.kind) {

		case '{':
		{
			if(!token_iterator_next(iterator)) {

				// #ERROR
				// Unexpected end of source in compound statement
				string_builder_append(output_builder, "Unexpected end of source inside a compound statement, right after the [{]. Was expected [}] or a statement");
				return 0;
			}

			int comp_stmt_offset,
				comp_stmt_length;

			comp_stmt_offset = token.offset;

			node_t *stmt_head = 0, 
		   		   *stmt_tail = 0;
		   	int stmt_count = 0;

			while(1) {

				node_t *stmt = parse_statement(pool, iterator, source, source_length, output_builder);

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

					string_builder_append(output_builder, "Unexpected end of source inside a compound statement. Was expected [}] or a statement");
					return 0;
				}

				token = token_iterator_current(iterator);

				if(token.kind == '}')
					break;
			}

			comp_stmt_length = token.offset + token.length - comp_stmt_offset;

			return node_compound_create(pool, comp_stmt_offset, comp_stmt_length, stmt_head, stmt_tail, stmt_count);
		}

		case TOKEN_KIND_KWORD_IMPORT:
		{

			int import_stmt_offset, import_stmt_length;

			import_stmt_offset = token.offset;

			if(!token_iterator_next(iterator)) {

				// #ERROR

				FAILED;

				string_builder_append(output_builder, "Unexpected end of source inside an import statement, right after the import statement. Was expected an expression as path");
				return 0;
			}

			node_t *expression = parse_expression(pool, iterator, source, source_length, output_builder);

			if(expression == 0)
				return 0;

			char *name = 0;

			if(!token_iterator_next(iterator)) {

				// #ERROR

				FAILED;

				string_builder_append(output_builder, "Unexpected end of source inside an import statement, after path expression. Was expected [as] or [;]");
				return 0;
			}

			token = token_iterator_current(iterator);

			if(token.kind == TOKEN_KIND_KWORD_AS) {

				if(!token_iterator_next(iterator)) {

					// #ERROR

					FAILED;

					string_builder_append(output_builder, "Unexpected end of source inside an import statement, after [as] keyword. Was expected an identifier");
					return 0;
				}

				token = token_iterator_current(iterator);

				if(token.kind != TOKEN_KIND_IDENTIFIER) {

					// #ERROR

					FAILED;

					string_builder_append(output_builder, "Unexpected token [${string-with-length}] inside an import statement, after [as] keyword. Was expected an identifier", source + token.offset, token.length);
					print_unexpected_token_location(output_builder, source, source_length, token);
					return 0;
				}

				if(!token_to_string(pool, token, source, &name, 0))
					return 0;

				if(!token_iterator_next(iterator)) {

					// #ERROR

					FAILED;

					string_builder_append(output_builder, "Unexpected end of source inside an import statement, after identifier. Was expected [;]");
					return 0;
				}

				token = token_iterator_current(iterator);
			}

			if(token.kind != ';') {

				string_builder_append(output_builder, "Unexpected token [${string-with-length}] inside an import statement. Was expected [;]", source + token.offset, token.length);
				print_unexpected_token_location(output_builder, source, source_length, token);
				return 0;
			}

			import_stmt_length = token.offset + token.length - import_stmt_offset;

			return node_import_create(pool, import_stmt_offset, import_stmt_length, expression, name);
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
			node_t *expression = parse_expression(pool, iterator, source, source_length, output_builder);
			
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after expression. Was expected [;]

				string_builder_append(output_builder, "Unexpected end of source after expression. Was expected [;]");
				print_missing_semicolon_location(output_builder, source, source_length, token);
				return 0;
			}
			
			token = token_iterator_current(iterator);

			if(token.kind != ';') {

				FAILED;

				// #ERROR
				// Unexpected token after expression. Was expected [;]

				string_builder_append(output_builder, "Unexpected token [${string-with-length}] after expression. Was expected [;]", source + token.offset, token.length);
				print_unexpected_token_location(output_builder, source, source_length, token);
				return 0;
			}

			return expression;
		}

		case TOKEN_KIND_KWORD_BREAK:
		{
			node_t *node = node_break_create(pool, token.offset, token.length);
		
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after break statement. Was expected [;]
				string_builder_append(output_builder, "Unexpected end of source after break statement. Was expected [;]");
				return 0;
			}
			
			token = token_iterator_current(iterator);

			if(token.kind != ';') {

				FAILED;

				// #ERROR
				// Unexpected token after break statement. Was expected [;]

				string_builder_append(output_builder, "Unexpected token [${string-with-length}] after break statement. Was expected [;]", source + token.offset, token.length);
				print_unexpected_token_location(output_builder, source, source_length, token);
				return 0;
			}

			return node;
		}

		case TOKEN_KIND_KWORD_CONTINUE:
		{
			node_t *node = node_continue_create(pool, token.offset, token.length);
		
			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after continue statement. Was expected [;]

				string_builder_append(output_builder, "Unexpected end of source after continue statement. Was expected [;]");
				return 0;
			}
			
			token = token_iterator_current(iterator);

			if(token.kind != ';') {

				FAILED;

				// #ERROR
				// Unexpected token after continue statement. Was expected [;]

				string_builder_append(output_builder, "Unexpected token [${string-with-length}] after continue statement. Was expected [;]", source + token.offset, token.length);
				print_unexpected_token_location(output_builder, source, source_length, token);
				return 0;
			}

			return node;
		}

		case TOKEN_KIND_KWORD_ELSE:
		{
			FAILED;

			// #ERROR
			// Unexpected else keyword. An else statement should follow an if statement
			
			string_builder_append(output_builder, "Unexpected token else statement. An else statement must come after an if statement!");
			print_unexpected_token_location(output_builder, source, source_length, token);
			return 0;
		}

		case TOKEN_KIND_KWORD_IF:
		return parse_ifelse_statement(pool, iterator, source, source_length, output_builder);

		case TOKEN_KIND_KWORD_WHILE:
		return parse_while_statement(pool, iterator, source, source_length, output_builder);

		case TOKEN_KIND_KWORD_RETURN:
		{
			int return_stmt_offset,
				return_stmt_length;

			return_stmt_offset = token.offset;

			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after return keyword. Was expected an expression

				string_builder_append(output_builder, "Unexpected end of source after return keyword. Was expected and expression");
				return 0;
			}

			node_t *expression = parse_expression(pool, iterator, source, source_length, output_builder);
			
			if(expression == 0)
				return 0;

			if(!token_iterator_next(iterator)) {

				FAILED;

				// #ERROR
				// Unexpected end of source after expression. Was expected [;]
				string_builder_append(output_builder, "Unexpected end of source after return statement value expression. Was expected [;]");
				return 0;
			}

			token = token_iterator_current(iterator);

			if(token.kind != ';') {

				FAILED;

				// #ERROR
				// Unexpected token after return statement. Was expected [;]

				string_builder_append(output_builder, "Unexpected token [${string-with-length}] after return statement value expression. Was expected [;]", source + token.offset, token.length);
				print_unexpected_token_location(output_builder, source, source_length, token);
				return 0;
			}

			return_stmt_length = token.offset + token.length - return_stmt_offset;

			return node_return_create(pool, return_stmt_offset, return_stmt_length, expression);
		}

		default:
		FAILED;

		// #ERROR
		// Unexpected token at the start of statement

		string_builder_append(output_builder, "Unexpected token at the start of a statement\n", source + token.offset, token.length);
		print_unexpected_token_location(output_builder, source, source_length, token);
		return 0;
	}

	return 0; //??
}

int parse(const char *source, int source_length, ast_t *e_ast, string_builder_t *output_builder)
{
	token_array_t array;

	if(!tokenize(source, source_length, &array)) {
		string_builder_append(output_builder, "Out of memory");
		return 0;
	}

	token_iterator_t iterator;
	token_iterator_init(&iterator, &array);

	pool_t *pool = pool_create();

	if(pool == 0)
		return 0;

	node_t *stmt_head = 0, 
		   *stmt_tail = 0;
	int 	stmt_count = 0;

	while(1) {

		node_t *stmt = parse_statement(pool, &iterator, source, source_length, output_builder);

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

	if(!check(result, source, source_length, output_builder)) {

		pool_destroy(pool);
		return 0;
	}

	e_ast->pool = pool;
	e_ast->root = result;
	return 1;
}