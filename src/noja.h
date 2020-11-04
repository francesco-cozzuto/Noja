
#include <stdint.h>
#include <stdio.h>

#include "pool.h"
#include "ast.h"
#include "token.h"

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

	executable_t *executable;
	
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

	uint32_t program_counters[1024];
	uint32_t program_counters_depth;

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

	void (*on_print)(state_t *state, object_t *self, FILE *fp);

	object_t *(*on_add)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_sub)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_mul)(state_t *state, object_t *self, object_t *right);
	object_t *(*on_div)(state_t *state, object_t *self, object_t *right);

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

	char *value;
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

extern object_bool_t object_true;
extern object_bool_t object_false;
extern object_type_t int_type_object;
extern object_type_t bool_type_object;
extern object_type_t dict_type_object;
extern object_type_t type_type_object;
extern object_type_t array_type_object;
extern object_type_t string_type_object;

object_t *dict_cselect(state_t *state, object_t *self, const char *name);
int 	  dict_cinsert(state_t *state, object_t *self, const char *name, object_t *value);
object_t *array_cselect(state_t *state, object_t *self, int64_t index);
int 	  array_cinsert(state_t *state, object_t *self, int64_t index, object_t *value);

object_t *object_from_cint(state_t *state, int64_t value);
object_t *object_istanciate(state_t *state, object_t *type);
void 	  object_print(state_t *state, object_t *self, FILE *fp);
object_t *object_add(state_t *state, object_t *self, object_t *right);

int  state_init(state_t *state, executable_t *executable);
void state_deinit(state_t *state);
int  step(state_t *state, char *error_buffer, int error_buffer_size);

int 	gc_requires_collection(state_t *state);
void   *gc_allocate(state_t *state, uint32_t size);
int 	gc_collect(state_t *state);

int tokenize(char *source, int source_length, token_array_t *e_token_array);
int parse(token_array_t *array, char *source, pool_t **e_pool, node_t **e_node);
int check(node_t *node, char *source, char *error_buffer, int error_buffer_size);
executable_t *generate(node_t *node);
void disassemble(executable_t *executable, FILE *fp);
executable_t *compile(char *source, int length, char *error_buffer, int error_buffer_size);
executable_t *compile_from_file(char *path, char *error_buffer, int error_buffer_size);

int get_lineno_of_offset(const char *text, int offset);
void report(char *error_buffer, int error_buffer_size, const char *format, ...);
int read_line(FILE *src, char **e_buffer, int *e_length);
int tokenize_buffer(char *buffer, char ***e_tokens, int *e_token_count);
int load_text(const char *path, char **e_content, int *e_length);

const char *get_instruction_operands(int opcode);
const char *get_opcode_name(int opcode);
