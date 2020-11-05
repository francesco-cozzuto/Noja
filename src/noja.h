
#include <stdint.h>
#include <stdio.h>

#include "utils/pool.h"
#include "ast.h"
#include "token.h"

enum {

	OPCODE_NOPE,
	OPCODE_QUIT,
		
	OPCODE_PUSH_NULL,
	OPCODE_PUSH_TRUE,
	OPCODE_PUSH_FALSE,
	OPCODE_PUSH_INT,
	OPCODE_PUSH_FLOAT,
	OPCODE_PUSH_STRING,
	OPCODE_PUSH_ARRAY,
	OPCODE_PUSH_DICT,
	OPCODE_PUSH_FUNCTION,
	OPCODE_PUSH_VARIABLE,

	OPCODE_POP,

	OPCODE_ASSIGN,
	OPCODE_SELECT,
	OPCODE_INSERT,
	OPCODE_SELECT_ATTRIBUTE,
	OPCODE_INSERT_ATTRIBUTE,

	OPCODE_VARIABLE_MAP_PUSH,
	OPCODE_VARIABLE_MAP_POP,

	OPCODE_BREAK,
	OPCODE_BREAK_DESTINATION_PUSH,
	OPCODE_BREAK_DESTINATION_POP,

	OPCODE_CONTINUE,
	OPCODE_CONTINUE_DESTINATION_PUSH,
	OPCODE_CONTINUE_DESTINATION_POP,

	OPCODE_CALL,
	OPCODE_EXPECT,
	OPCODE_RETURN,

	OPCODE_JUMP_ABSOLUTE,
	OPCODE_JUMP_IF_FALSE_AND_POP,

	OPCODE_ADD,
	OPCODE_SUB,
	OPCODE_MUL,
	OPCODE_DIV,
	OPCODE_MOD,
	OPCODE_POW,
	OPCODE_NEG,
	OPCODE_LSS,
	OPCODE_GRT,
	OPCODE_LEQ,
	OPCODE_GEQ,
	OPCODE_EQL,
	OPCODE_NQL,
	OPCODE_AND,
	OPCODE_OR,
	OPCODE_NOT,
	OPCODE_SHL,
	OPCODE_SHR,
	OPCODE_BITWISE_AND,
	OPCODE_BITWISE_OR,
	OPCODE_BITWISE_XOR,
	OPCODE_BITWISE_NOT,
	
};

typedef struct object_t object_t;

struct object_t {
	object_t *new_location;
	object_t *type;
	uint32_t flags;
};

typedef struct {

	char *data, *code;
	uint32_t data_length, code_length;
} executable_t;

typedef struct overflow_allocation_t overflow_allocation_t;
struct overflow_allocation_t {
	overflow_allocation_t *prev;
	char body[];
};

typedef struct {

	char *heap;
	uint32_t heap_size;
	uint32_t heap_used;
	overflow_allocation_t *overflow_allocations;

	object_t **stack;
	uint32_t stack_item_count;
	uint32_t stack_item_count_max;

	object_t **variable_maps;
	uint32_t variable_maps_count;
	uint32_t variable_maps_count_max;

	uint32_t break_destinations[16];
	uint32_t break_destinations_depth;

	uint32_t continue_destinations[16];
	uint32_t continue_destinations_depth;

	executable_t *executable_stack[16];
	uint32_t program_counters[16];
	uint32_t call_depth;

	int64_t argc;

} state_t;

typedef struct {

	object_t super;

	const char *name;
	size_t 		size;
	
	object_t *methods;

	int (*on_init)  (state_t *state, object_t *self);
	int (*on_deinit)(state_t *state, object_t *self);

	object_t *(*on_select)(state_t *state, object_t *self, object_t *key);
	int       (*on_insert)(state_t *state, object_t *self, object_t *key, object_t *value);

	object_t *(*on_select_attribute)(state_t *state, object_t *self, const char *name);
	int       (*on_insert_attribute)(state_t *state, object_t *self, const char *name, object_t *value);

	void (*on_print)(state_t *state, object_t *self, FILE *fp);

	object_t *(*on_add)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_sub)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_mul)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_div)(state_t *state, object_t *self, object_t *right);

	uint8_t (*on_test)(state_t *state, object_t *self);

} object_type_t;

typedef struct {

	object_t super;

	int64_t value;

} object_int_t;

typedef struct {

	object_t super;

	double value;

} object_float_t;

typedef struct {
	object_t super;
	int flags;
	union {
		char *value;
		const char *ref_value;
	};
	size_t length;
} object_string_t;

typedef struct {

	object_t super;
	
	int *map;
	int  map_size;

	char     **item_keys;
	object_t **item_values;

	int item_size;
	int item_used;

} object_dict_t;

typedef struct {

	object_t super;

	object_t **items;

	int item_size;
	int item_used;

} object_array_t;

typedef struct {
	object_t super;
	uint8_t value;
} object_bool_t;

typedef struct {
	object_t super;
	executable_t *executable;
	uint32_t offset;
} object_function_t;

typedef struct {
	object_t super;
	object_t *(*routine)(state_t *state, int argc, object_t **argv);
} object_cfunction_t;

extern object_t object_null;
extern object_bool_t object_true;
extern object_bool_t object_false;
extern object_type_t int_type_object;
extern object_type_t null_type_object;
extern object_type_t bool_type_object;
extern object_type_t dict_type_object;
extern object_type_t type_type_object;
extern object_type_t array_type_object;
extern object_type_t float_type_object;
extern object_type_t string_type_object;
extern object_type_t function_type_object;
extern object_type_t cfunction_type_object;

int insert_builtins(state_t *state, object_t *dest, char *error_buffer, int error_buffer_size);

object_t *dict_cselect(state_t *state, object_t *self, const char *name);
int 	  dict_cinsert(state_t *state, object_t *self, const char *name, object_t *value);
object_t *array_cselect(state_t *state, object_t *self, int64_t index);
int 	  array_cinsert(state_t *state, object_t *self, int64_t index, object_t *value);

object_t *object_from_cint(state_t *state, int64_t value);
object_t *object_from_cfloat(state_t *state, double value);
object_t *object_from_cstring(state_t *state, char *value, size_t length);
object_t *object_from_cstring_ref(state_t *state, const char *value, size_t length);
object_t *object_from_cfunction(state_t *state, object_t *(*routine)(state_t *state, int argc, object_t **argv));
object_t *object_from_executable_and_offset(state_t *state, executable_t *executable, uint32_t offset);
object_t *object_istanciate(state_t *state, object_t *type);
void 	  object_print(state_t *state, object_t *self, FILE *fp);
object_t *object_add(state_t *state, object_t *self, object_t *right);
uint8_t   object_test(state_t *state, object_t *object);
object_t *object_select(state_t *state, object_t *self, object_t *key);
int 	  object_insert(state_t *state, object_t *self, object_t *key, object_t *item);
object_t *object_select_attribute(state_t *state, object_t *self, const char *name);
int 	  object_insert_attribute(state_t *state, object_t *self, const char *name, object_t *value);

int tokenize(char *source, int source_length, token_array_t *e_token_array);
int parse(token_array_t *array, char *source, pool_t **e_pool, node_t **e_node);
int check(node_t *node, char *source, char *error_buffer, int error_buffer_size);
executable_t *generate(node_t *node);

int run_text(const char *text, int length, char *error_buffer, int error_buffer_size);
int run_file(const char *path, char *error_buffer, int error_buffer_size);

int get_lineno_of_offset(const char *text, int offset);
void report(char *error_buffer, int error_buffer_size, const char *format, ...);

