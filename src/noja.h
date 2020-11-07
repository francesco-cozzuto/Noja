
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "utils/string_builder.h"
#include "ast.h"

enum {

	OPCODE_NOPE,
	OPCODE_QUIT,
		
	OPCODE_PUSH_NULL,
	OPCODE_PUSH_TRUE,
	OPCODE_PUSH_FALSE,
	OPCODE_PUSH_INT,
	OPCODE_PUSH_FLOAT,
	OPCODE_PUSH_STRING,
	OPCODE_PUSH_FUNCTION,
	OPCODE_PUSH_VARIABLE,
	OPCODE_SELECT_ATTRIBUTE_AND_REPUSH,

	OPCODE_BUILD_ARRAY,
	OPCODE_BUILD_DICT,

	OPCODE_POP,

	OPCODE_IMPORT,
	OPCODE_IMPORT_AS,

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

typedef struct state_t state_t;
typedef struct object_t object_t;

struct object_t {
	object_t *new_location;
	object_t *type;
	uint32_t flags;
};

typedef struct {
	object_t super;
	uint8_t value;
} object_bool_t;

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
	object_t *(*on_mod)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_pow)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_lss)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_grt)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_leq)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_geq)(state_t *state, object_t *self, object_t *right);

	object_t *(*on_eql)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_nql)(state_t *state, object_t *self, object_t *right);
	
	object_t *(*on_and)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_or)(state_t *state, object_t *self, object_t *right);

	object_t *(*on_bitwise_and)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_bitwise_or)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_bitwise_xor)(state_t *state, object_t *self, object_t *right);

	object_t *(*on_shl)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_shr)(state_t *state, object_t *self, object_t *right);

	uint8_t (*on_test)(state_t *state, object_t *self);

} object_type_t;

typedef struct overflow_allocation_t overflow_allocation_t;
struct overflow_allocation_t {
	overflow_allocation_t *prev;
	char body[];
};

typedef struct {
	char *data;
	char *code;
	uint32_t data_size;
	uint32_t code_size;
	object_t *global_variables_map;
} segment_t;

#define OBJECT_STACK_ITEMS_PER_CHUNK 128
#define U32_STACK_ITEMS_PER_CHUNK 128

typedef struct object_stack_chunk_t object_stack_chunk_t;
struct object_stack_chunk_t {
	object_stack_chunk_t *prev;
	object_t *items[OBJECT_STACK_ITEMS_PER_CHUNK];
};

typedef struct {
	object_stack_chunk_t head, *tail;
	uint32_t relative_size;
	uint32_t absolute_size;
} object_stack_t;

typedef struct u32_stack_chunk_t u32_stack_chunk_t;
struct u32_stack_chunk_t {
	u32_stack_chunk_t *prev;
	uint32_t items[U32_STACK_ITEMS_PER_CHUNK];
};

typedef struct {
	u32_stack_chunk_t head, *tail;
	uint32_t relative_size;
	uint32_t absolute_size;
} u32_stack_t;

struct state_t {

	object_t null_object;
	object_bool_t true_object;
	object_bool_t false_object;

	object_type_t type_object_int;
	object_type_t type_object_dict;
	object_type_t type_object_bool;
	object_type_t type_object_null;
	object_type_t type_object_type;
	object_type_t type_object_array;
	object_type_t type_object_float;
	object_type_t type_object_string;
	object_type_t type_object_function;
	object_type_t type_object_cfunction;

	int failed;
	int64_t argc;

	string_builder_t *output_builder;

	char *heap;
	uint32_t heap_size;
	uint32_t heap_used;
	overflow_allocation_t *overflow_allocations;

	object_stack_t eval_stack;
	object_stack_t vars_stack;
	object_t *builtins_map;

	// Break/continue stuff

	u32_stack_t break_destinations;
	u32_stack_t continue_destinations;

	// Keep track of the point of execution

	u32_stack_t segment_stack;
	u32_stack_t offset_stack;

	// Virtual memory simulation stuff

	segment_t *segments;
	int segments_size;
	int segments_used;
};

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
	uint32_t segment;
	uint32_t offset;
} object_function_t;

typedef struct {
	object_t super;
	object_t *(*routine)(state_t *state, int argc, object_t **argv);
} object_cfunction_t;

typedef struct {
	object_t super;
	object_t *dict;
} object_module_t;

typedef object_t *(*builtin_interface_t)(state_t *state, int argc, object_t **argv);


void 	   object_stack_init(object_stack_t *stack);
void 	   object_stack_deinit(object_stack_t *stack);
int 	   object_stack_size(object_stack_t *stack);
int 	   object_push(object_stack_t *stack, object_t *item);
object_t  *object_pop(object_stack_t *stack);
object_t  *object_top(object_stack_t *stack);
object_t **object_top_ref(object_stack_t *stack);
object_t  *object_nth_from_top(object_stack_t *stack, int count);
void 	   object_stack_print(state_t *state, object_stack_t *stack, FILE *fp);

void 	  u32_stack_init(u32_stack_t *stack);
int 	  u32_stack_size(u32_stack_t *stack);
void 	  u32_stack_deinit(u32_stack_t *stack);
int 	  u32_push(u32_stack_t *stack, uint32_t item);
uint32_t  u32_pop(u32_stack_t *stack);
uint32_t  u32_top(u32_stack_t *stack);
uint32_t *u32_top_ref(u32_stack_t *stack);

int 	  dict_import(state_t *state, object_t *self, object_t *other);
object_t *dict_cselect(state_t *state, object_t *self, const char *name);
int 	  dict_cinsert(state_t *state, object_t *self, const char *name, object_t *value);
object_t *array_cselect(state_t *state, object_t *self, int64_t index);
int 	  array_cinsert(state_t *state, object_t *self, int64_t index, object_t *value);

object_t *object_from_cint(state_t *state, int64_t value);
object_t *object_from_cfloat(state_t *state, double value);
object_t *object_from_cstring(state_t *state, char *value, size_t length);
object_t *object_from_cstring_ref(state_t *state, const char *value, size_t length);
object_t *object_from_cfunction(state_t *state, object_t *(*routine)(state_t *state, int argc, object_t **argv));
object_t *object_from_segment_and_offset(state_t *state, uint32_t segment, uint32_t offset);
object_t *object_istanciate(state_t *state, object_t *type);
void 	  object_print(state_t *state, object_t *self, FILE *fp);

object_t *object_add(state_t *state, object_t *self, object_t *right);
object_t *object_sub(state_t *state, object_t *self, object_t *right);
object_t *object_mul(state_t *state, object_t *self, object_t *right);
object_t *object_div(state_t *state, object_t *self, object_t *right);
object_t *object_mod(state_t *state, object_t *self, object_t *right);
object_t *object_pow(state_t *state, object_t *self, object_t *right);

object_t *object_lss(state_t *state, object_t *self, object_t *right);
object_t *object_grt(state_t *state, object_t *self, object_t *right);
object_t *object_leq(state_t *state, object_t *self, object_t *right);
object_t *object_geq(state_t *state, object_t *self, object_t *right);

object_t *object_eql(state_t *state, object_t *self, object_t *right);
object_t *object_nql(state_t *state, object_t *self, object_t *right);

object_t *object_and(state_t *state, object_t *self, object_t *right);
object_t *object_or (state_t *state, object_t *self, object_t *right);

object_t *object_bitwise_and(state_t *state, object_t *self, object_t *right);
object_t *object_bitwise_or (state_t *state, object_t *self, object_t *right);
object_t *object_bitwise_xor(state_t *state, object_t *self, object_t *right);

object_t *object_shl(state_t *state, object_t *self, object_t *right);
object_t *object_shr(state_t *state, object_t *self, object_t *right);

uint8_t   object_test(state_t *state, object_t *object);
object_t *object_select(state_t *state, object_t *self, object_t *key);
int 	  object_insert(state_t *state, object_t *self, object_t *key, object_t *item);
object_t *object_select_attribute(state_t *state, object_t *self, const char *name);
int 	  object_insert_attribute(state_t *state, object_t *self, const char *name, object_t *value);

int run_text(const char *text, int length, char **error_text);
int run_file(const char *path, char **error_text);

int parse(const char *source, int source_length, ast_t *ast, string_builder_t *output_builder);
int generate(ast_t ast, char **e_data, char **e_code, uint32_t *e_data_size, uint32_t *e_code_size);