
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "utils/string_builder.h"

typedef struct nj_state_t nj_state_t;
typedef struct nj_object_t nj_object_t;

struct nj_object_t {
	nj_object_t *new_location;
	nj_object_t *type;
	uint32_t flags;
};

typedef struct {
	nj_object_t super;
	uint8_t value;
} nj_object_bool_t;

typedef struct {

	nj_object_t super;

	const char *name;
	size_t 		size;
	
	nj_object_t *methods;

	int (*on_init)  (nj_state_t *state, nj_object_t *self);
	int (*on_deinit)(nj_state_t *state, nj_object_t *self);

	nj_object_t *(*on_select)(nj_state_t *state, nj_object_t *self, nj_object_t *key);
	int          (*on_insert)(nj_state_t *state, nj_object_t *self, nj_object_t *key, nj_object_t *value);

	nj_object_t *(*on_select_attribute)(nj_state_t *state, nj_object_t *self, const char *name);
	int          (*on_insert_attribute)(nj_state_t *state, nj_object_t *self, const char *name, nj_object_t *value);

	void (*on_print)(nj_state_t *state, nj_object_t *self, FILE *fp);

	nj_object_t *(*on_add)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_sub)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_mul)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_div)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_mod)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_pow)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_lss)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_grt)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_leq)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_geq)(nj_state_t *state, nj_object_t *self, nj_object_t *right);

	nj_object_t *(*on_eql)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_nql)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	
	nj_object_t *(*on_and)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_or)(nj_state_t *state, nj_object_t *self, nj_object_t *right);

	nj_object_t *(*on_bitwise_and)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_bitwise_or)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_bitwise_xor)(nj_state_t *state, nj_object_t *self, nj_object_t *right);

	nj_object_t *(*on_shl)(nj_state_t *state, nj_object_t *self, nj_object_t *right);
	nj_object_t *(*on_shr)(nj_state_t *state, nj_object_t *self, nj_object_t *right);

	uint8_t (*on_test)(nj_state_t *state, nj_object_t *self);

} nj_object_type_t;

typedef struct overflow_allocation_t overflow_allocation_t;
struct overflow_allocation_t {
	overflow_allocation_t *prev;
	char body[];
};

typedef struct {
	char *name;
	char *text;
	char *data;
	char *code;
	uint32_t data_size;
	uint32_t code_size;
	nj_object_t *global_variables_map;
} segment_t;

#define OBJECT_STACK_ITEMS_PER_CHUNK 128
#define U32_STACK_ITEMS_PER_CHUNK 128

typedef struct object_stack_chunk_t object_stack_chunk_t;
struct object_stack_chunk_t {
	object_stack_chunk_t *prev;
	nj_object_t *items[OBJECT_STACK_ITEMS_PER_CHUNK];
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

struct nj_state_t {

	nj_object_t 	 null_object;
	nj_object_bool_t true_object;
	nj_object_bool_t false_object;

	nj_object_type_t type_object_int;
	nj_object_type_t type_object_dict;
	nj_object_type_t type_object_bool;
	nj_object_type_t type_object_null;
	nj_object_type_t type_object_type;
	nj_object_type_t type_object_array;
	nj_object_type_t type_object_float;
	nj_object_type_t type_object_string;
	nj_object_type_t type_object_function;
	nj_object_type_t type_object_cfunction;

	int failed;
	int64_t argc;

	string_builder_t *output_builder;

	char *heap;
	uint32_t heap_size;
	uint32_t heap_used;
	overflow_allocation_t *overflow_allocations;

	object_stack_t eval_stack;
	object_stack_t vars_stack;
	nj_object_t *builtins_map;
	u32_stack_t segment_stack;
	u32_stack_t offset_stack;

	// Virtual memory simulation stuff

	segment_t *segments;
	int segments_size;
	int segments_used;
};

typedef struct {

	nj_object_t super;

	int64_t value;

} nj_object_int_t;

typedef struct {

	nj_object_t super;

	double value;

} nj_object_float_t;

typedef struct {
	nj_object_t super;
	int flags;
	union {
		char *value;
		const char *ref_value;
	};
	size_t length;
} nj_object_string_t;

typedef struct {

	nj_object_t super;
	
	int *map;
	int  map_size;

	char     **item_keys;
	nj_object_t **item_values;

	int item_size;
	int item_used;

} nj_object_dict_t;

typedef struct {

	nj_object_t super;

	nj_object_t **items;

	int item_size;
	int item_used;

} nj_object_array_t;

typedef struct {
	nj_object_t super;
	uint32_t segment;
	uint32_t offset;
} nj_object_function_t;

typedef struct {
	nj_object_t super;
	nj_object_t *(*routine)(nj_state_t *state, int argc, nj_object_t **argv);
} nj_object_cfunction_t;

typedef struct {
	nj_object_t super;
	nj_object_t *dict;
} nj_object_module_t;

typedef nj_object_t *(*builtin_interface_t)(nj_state_t *state, int argc, nj_object_t **argv);

void 	      object_stack_init(object_stack_t *stack);
void 	      object_stack_deinit(object_stack_t *stack);
int 	   	  object_stack_size(object_stack_t *stack);
int 	   	  object_push(object_stack_t *stack, nj_object_t *item);
nj_object_t  *object_pop(object_stack_t *stack);
nj_object_t  *object_top(object_stack_t *stack);
nj_object_t **object_top_ref(object_stack_t *stack);
nj_object_t  *object_nth_from_top(object_stack_t *stack, int count);
void 	      object_stack_print(nj_state_t *state, object_stack_t *stack, FILE *fp);

void 	  u32_stack_init(u32_stack_t *stack);
int 	  u32_stack_size(u32_stack_t *stack);
void 	  u32_stack_deinit(u32_stack_t *stack);
int 	  u32_push(u32_stack_t *stack, uint32_t item);
uint32_t  u32_pop(u32_stack_t *stack);
uint32_t  u32_top(u32_stack_t *stack);
uint32_t *u32_top_ref(u32_stack_t *stack);

int 	  	 nj_dictionary_merge_in(nj_state_t *state, nj_object_t *self, nj_object_t *other);
nj_object_t *nj_dictionary_select(nj_state_t *state, nj_object_t *self, const char *name);
int 	  	 nj_dictionary_insert(nj_state_t *state, nj_object_t *self, const char *name, nj_object_t *value);

nj_object_t *nj_array_select(nj_state_t *state, nj_object_t *self, int64_t index);
int 	     nj_array_insert(nj_state_t *state, nj_object_t *self, int64_t index, nj_object_t *value);

int nj_object_to_c_int(nj_state_t *state, nj_object_t *object, int64_t *value);
int nj_object_to_c_float(nj_state_t *state, nj_object_t *object, double *value);
int nj_object_to_c_string(nj_state_t *state, nj_object_t *object, const char **value, int *length);

nj_object_t *nj_object_from_c_int(nj_state_t *state, int64_t value);
nj_object_t *nj_object_from_c_float(nj_state_t *state, double value);
nj_object_t *nj_object_from_c_string(nj_state_t *state, char *value, size_t length);
nj_object_t *nj_object_from_c_string_ref(nj_state_t *state, const char *value, size_t length);
nj_object_t *nj_object_from_c_string_ref_2(nj_state_t *state, const char *value, size_t length);
nj_object_t *nj_object_from_c_function(nj_state_t *state, nj_object_t *(*routine)(nj_state_t *state, int argc, nj_object_t **argv));
nj_object_t *nj_object_from_segment_and_offset(nj_state_t *state, uint32_t segment, uint32_t offset);
nj_object_t *nj_object_istanciate(nj_state_t *state, nj_object_t *type);
void 	     nj_object_print(nj_state_t *state, nj_object_t *self, FILE *fp);
nj_object_t *nj_object_type(nj_object_t *self);
nj_object_t *nj_object_add(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_sub(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_mul(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_div(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_mod(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_pow(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_lss(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_grt(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_leq(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_geq(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_eql(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_nql(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_and(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_or (nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_bitwise_and(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_bitwise_or (nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_bitwise_xor(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_shl(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_shr(nj_state_t *state, nj_object_t *self, nj_object_t *right);
uint8_t   	 nj_object_test(nj_state_t *state, nj_object_t *object);

nj_object_t *nj_object_select(nj_state_t *state, nj_object_t *self, nj_object_t *key);
int 	  	 nj_object_insert(nj_state_t *state, nj_object_t *self, nj_object_t *key, nj_object_t *item);
nj_object_t *nj_object_select_attribute(nj_state_t *state, nj_object_t *self, const char *name);
int 	  	 nj_object_insert_attribute(nj_state_t *state, nj_object_t *self, const char *name, nj_object_t *value);

nj_object_t *nj_get_dict_type_object(nj_state_t *state);
nj_object_t *nj_get_int_type_object(nj_state_t *state);
nj_object_t *nj_get_float_type_object(nj_state_t *state);
nj_object_t *nj_get_bool_type_object(nj_state_t *state);
nj_object_t *nj_get_array_type_object(nj_state_t *state);
nj_object_t *nj_get_type_type_object(nj_state_t *state);
nj_object_t *nj_get_string_type_object(nj_state_t *state);
nj_object_t *nj_get_null_type_object(nj_state_t *state);
nj_object_t *nj_get_function_type_object(nj_state_t *state);
nj_object_t *nj_get_cfunction_type_object(nj_state_t *state);
nj_object_t *nj_get_null_object(nj_state_t *state);
nj_object_t *nj_get_true_object(nj_state_t *state);
nj_object_t *nj_get_false_object(nj_state_t *state);

void nj_fail(nj_state_t *state, const char *fmt, ...);
int  nj_failed(nj_state_t *state);

int nj_run(const char *text, int length, char **error_text);
int nj_run_file(const char *path, char **error_text);

void nj_disassemble(char *code, char *data, uint32_t code_size, uint32_t data_size);
int nj_compile(const char *text, size_t length, char **e_data, char **e_code, uint32_t *e_data_size, uint32_t *e_code_size, string_builder_t *output_builder);

int nj_import(nj_state_t *state);
int nj_import_as(nj_state_t *state, const char *name);

int  nj_state_init(nj_state_t *state, string_builder_t *output_builder);
void nj_state_deinit(nj_state_t *state);
int  nj_step(nj_state_t *state);

int append_segment(nj_state_t *state, char *code, char *data, uint32_t code_size, uint32_t data_size, uint32_t *e_segment);