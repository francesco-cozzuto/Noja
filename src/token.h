
#include <stdint.h>
#include <stdio.h>
#include "pool.h"

typedef struct {
	int kind;
	int offset;
	int length;
} token_t;

#define TOKENS_PER_CHUNK 1028

typedef struct token_chunk_t token_chunk_t;
struct token_chunk_t {
	token_chunk_t *prev, *next;
	int used;
	token_t tokens[TOKENS_PER_CHUNK];
};

typedef struct {
	token_chunk_t head, *tail;
	int count;
} token_array_t;

typedef struct {
	token_array_t *array;
	token_chunk_t *chunk;
	int count;
	int absolute_offset;
	int relative_offset;
} token_iterator_t;

enum {

	TOKEN_KIND_VALUE_INT = 256,
	TOKEN_KIND_VALUE_FLOAT,
	TOKEN_KIND_VALUE_STRING,

	TOKEN_KIND_KWORD_BREAK,
	TOKEN_KIND_KWORD_CONTINUE,
	TOKEN_KIND_KWORD_ELSE,
	TOKEN_KIND_KWORD_IF,
	TOKEN_KIND_KWORD_WHILE,
	TOKEN_KIND_KWORD_FUNCTION,
	TOKEN_KIND_KWORD_RETURN,
	TOKEN_KIND_IDENTIFIER,

	TOKEN_KIND_OPERATOR_ADD,
	TOKEN_KIND_OPERATOR_SUB,
	TOKEN_KIND_OPERATOR_MUL,
	TOKEN_KIND_OPERATOR_DIV,
	TOKEN_KIND_OPERATOR_MOD,
	TOKEN_KIND_OPERATOR_POW,

	TOKEN_KIND_OPERATOR_LSS,
	TOKEN_KIND_OPERATOR_GRT,
	TOKEN_KIND_OPERATOR_LEQ,
	TOKEN_KIND_OPERATOR_GEQ,

	TOKEN_KIND_OPERATOR_EQL,
	TOKEN_KIND_OPERATOR_NQL,

	TOKEN_KIND_OPERATOR_AND,
	TOKEN_KIND_OPERATOR_OR,
	TOKEN_KIND_OPERATOR_NOT,

	TOKEN_KIND_OPERATOR_BITWISE_AND,
	TOKEN_KIND_OPERATOR_BITWISE_OR,
	TOKEN_KIND_OPERATOR_BITWISE_XOR,
	TOKEN_KIND_OPERATOR_BITWISE_NOT,

	TOKEN_KIND_OPERATOR_SHL,
	TOKEN_KIND_OPERATOR_SHR,

	TOKEN_KIND_OPERATOR_INC,
	TOKEN_KIND_OPERATOR_DEC,

	TOKEN_KIND_OPERATOR_ASSIGN,
	TOKEN_KIND_OPERATOR_ASSIGN_ADD,
	TOKEN_KIND_OPERATOR_ASSIGN_SUB,
	TOKEN_KIND_OPERATOR_ASSIGN_MUL,
	TOKEN_KIND_OPERATOR_ASSIGN_DIV,
	TOKEN_KIND_OPERATOR_ASSIGN_MOD,
	TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_AND,
	TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_OR,
	TOKEN_KIND_OPERATOR_ASSIGN_BITWISE_XOR,
	TOKEN_KIND_OPERATOR_ASSIGN_SHL,
	TOKEN_KIND_OPERATOR_ASSIGN_SHR,

};

void token_array_init(token_array_t *array);
int  token_array_push(token_array_t *array, token_t token);
void token_array_print(token_array_t *array, char *source, FILE *fp);
void token_array_foreach(token_array_t *array, void *userdata, int (*callback)(void *data, int index, token_t token));
void token_array_deinit(token_array_t *array);


void token_iterator_init(token_iterator_t *iterator, token_array_t *array);
int __token_iterator_next(token_iterator_t *iterator, char *file, int line, const char *func, char *source);
int __token_iterator_prev(token_iterator_t *iterator, char *file, int line, const char *func, char *source);
#define token_iterator_next(iterator) __token_iterator_next(iterator, __FILE__, __LINE__, __func__, source)
#define token_iterator_prev(iterator) __token_iterator_prev(iterator, __FILE__, __LINE__, __func__, source)
//int  token_iterator_next(token_iterator_t *iterator);
//int  token_iterator_prev(token_iterator_t *iterator);
token_t token_iterator_current(token_iterator_t *iterator);

int64_t token_to_int(token_t token, char *text);
double 	token_to_float(token_t token, char *text);
int 	token_to_string(pool_t *pool, token_t token, char *text, char **e_result, int *e_length);