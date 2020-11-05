
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "noja.h"

#warning "Implement comments"

static char is_alpha(char c)
{
	return (c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A');
}

static char is_digit(char c)
{
	return (c <= '9' && c >= '0');
}

static char is_whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static char read_character(char *source, int source_length, int *current_offset)
{
	if(*current_offset == source_length)
		return '\0';

	return source[(*current_offset)++];
}

static void assign_identifier_kind(char *source, token_t *token)
{
	token->kind = -1;

	#define MATCH(e) (sizeof(e)-1 == token->length && !strncmp(e, source + token->offset, token->length))

	switch(source[token->offset]) {

		case 'b':
		if MATCH("break") {
			token->kind = TOKEN_KIND_KWORD_BREAK;
			break;
		}
		break;
		
		case 'c':
		if MATCH("continue") {
			token->kind = TOKEN_KIND_KWORD_CONTINUE;
			break;
		}
		break;

		case 'e':
		if MATCH("else") {
			token->kind = TOKEN_KIND_KWORD_ELSE;
			break;
		}
		break;

		case 'f':
		if MATCH("function") {
			token->kind = TOKEN_KIND_KWORD_FUNCTION;
			break;
		}
		break;

		case 'i':
		if MATCH("if") {
			token->kind = TOKEN_KIND_KWORD_IF;
			break;
		}
		break;

		case 'w':
		if MATCH("while") {
			token->kind = TOKEN_KIND_KWORD_WHILE;
			break;
		}
		break;

		case 'r':
		if MATCH("return") {
			token->kind = TOKEN_KIND_KWORD_RETURN;
			break;
		}
		break;
		
	}

	#undef MATCH

	if(token->kind < 0)
		token->kind = TOKEN_KIND_IDENTIFIER;
}

static char is_operator_character(char c)
{
	return c == '=' || c == '+' || c == '-' || c == '*' || 
		   c == '/' || c == '%' || c == '<' || c == '>' || 
		   c == '!' || c == '&' || c == '|' || c == '^' || c == '~';
}

static int get_operator_kind(char *string, int length)
{

	#define MATCH(e) (sizeof(e)-1 == length && !strncmp(e, string, length))

	switch(string[0]) {

		case '+':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_ADD;

		if MATCH("++")
			return TOKEN_KIND_OPERATOR_INC;

		if MATCH("+=")
			return TOKEN_KIND_OPERATOR_ASSIGN_ADD;

		break;

		case '-':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_SUB;

		if MATCH("--")
			return TOKEN_KIND_OPERATOR_DEC;

		if MATCH("-=")
			return TOKEN_KIND_OPERATOR_ASSIGN_SUB;

		break;

		case '*':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_MUL;

		if MATCH("*=")
			return TOKEN_KIND_OPERATOR_ASSIGN_MUL;

		if MATCH("**")
			return TOKEN_KIND_OPERATOR_POW;

		break;

		case '/':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_DIV;

		if MATCH("/=")
			return TOKEN_KIND_OPERATOR_ASSIGN_DIV;

		break;

		case '%':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_MOD;

		if MATCH("%=")
			return TOKEN_KIND_OPERATOR_ASSIGN_MOD;

		break;

		case '&':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_BITWISE_AND;

		if MATCH("&&")
			return TOKEN_KIND_OPERATOR_AND;

		if MATCH("&=")
			return TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_AND;

		break;

		case '|':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_BITWISE_OR;

		if MATCH("||")
			return TOKEN_KIND_OPERATOR_OR;

		if MATCH("|=")
			return TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_OR;

		break;

		case '^':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_BITWISE_XOR;

		if MATCH("^=")
			return TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_XOR;

		break;

		case '<':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_LSS;

		if MATCH("<=")
			return TOKEN_KIND_OPERATOR_LEQ;

		if MATCH("<<")
			return TOKEN_KIND_OPERATOR_SHL;

		if MATCH("<<=")
			return TOKEN_KIND_OPERATOR_ASSIGN_SHL;

		break;

		case '>':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_GRT;

		if MATCH(">=")
			return TOKEN_KIND_OPERATOR_GEQ;

		if MATCH(">>")
			return TOKEN_KIND_OPERATOR_SHR;

		if MATCH(">>=")
			return TOKEN_KIND_OPERATOR_ASSIGN_SHR;

		break;

		case '=':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_ASSIGN;

		if MATCH("==")
			return TOKEN_KIND_OPERATOR_EQL;

		break;

		case '!':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_NOT;

		if MATCH("!=")
			return TOKEN_KIND_OPERATOR_NQL;

		break;

		case '~':
		
		if(length == 1)
			return TOKEN_KIND_OPERATOR_BITWISE_NOT;

		break;
	}

	#undef MATCH

	return -1;
}

static int tokenize_one(char *source, int source_length, int *current_offset, token_t *e_token)
{
	char c;

	while(is_whitespace(c = read_character(source, source_length, current_offset)));

	if(c == '\0')
		return 0;

	e_token->offset = (*current_offset)-1;

	if(is_alpha(c) || c == '_') {

		while(is_alpha(c = read_character(source, source_length, current_offset)) || is_digit(c) || c == '_');
		(*current_offset)--;

		e_token->length = (*current_offset) - e_token->offset;

		assign_identifier_kind(source, e_token);

	} else if(is_digit(c)) {


		while(is_digit(c = read_character(source, source_length, current_offset)));

		if(c == '.') {

			while(is_digit(c = read_character(source, source_length, current_offset)));

			e_token->kind = TOKEN_KIND_VALUE_FLOAT;

		} else {

			e_token->kind = TOKEN_KIND_VALUE_INT;

		}

		(*current_offset)--;

		e_token->length = (*current_offset) - e_token->offset;


	} else if(c == '"') {

		e_token->kind = TOKEN_KIND_VALUE_STRING;

		while((c = read_character(source, source_length, current_offset)) != '"' && c != '\0');

		e_token->length = (*current_offset) - e_token->offset;

	} else if(is_operator_character(c)) {

		e_token->kind = get_operator_kind(source + e_token->offset, 1);

		while(1) {

			// check if the operator can be one character longer

			read_character(source, source_length, current_offset);

			int kind = get_operator_kind(source + e_token->offset, (*current_offset) - e_token->offset);

			if(kind == -1)
				break; // it can't

			// it can!

			e_token->kind = kind;

		}

		(*current_offset)--;

		e_token->length = (*current_offset) - e_token->offset;

		// Did we find an operator?

		if(e_token->kind == -1) {

			// Nope!

			e_token->kind = source[e_token->offset];
			e_token->length = 1;

			(*current_offset) = e_token->offset + 1;
		}

	} else {


		e_token->kind = c;
		e_token->length = 1;
	}

	return 1;
}

int tokenize(char *source, int source_length, token_array_t *e_token_array)
{
	token_array_init(e_token_array);

	int current_offset = 0;

	while(1) {

		token_t token;

		if(!tokenize_one(source, source_length, &current_offset, &token))
			break;

		if(!token_array_push(e_token_array, token)) {
			
			// #ERROR
			// Out of memory

			token_array_deinit(e_token_array);
			return 0;
		}

	}

	return 1;
}
