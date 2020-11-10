
#include <stdint.h>
#include <stdio.h>

// Types

typedef void nj_state_t;
typedef void nj_object_t;

void nj_fail(nj_state_t *state, const char *fmt, ...);
int  nj_failed(nj_state_t *state);

int nj_run(const char *text, int length, char *error_buffer, int error_buffer_size);
int nj_run_file(const char *path, char *error_buffer, int error_buffer_size);

// C to Noja types conversion functions

int nj_object_to_c_int(nj_state_t *state, nj_object_t *object, int64_t *value);
int nj_object_to_c_float(nj_state_t *state, nj_object_t *object, double *value);
int nj_object_to_c_string(nj_state_t *state, nj_object_t *object, const char **value, int *length);

nj_object_t *nj_object_from_c_int(nj_state_t *state, int64_t value);
nj_object_t *nj_object_from_c_bool(nj_state_t *state, unsigned char value);
nj_object_t *nj_object_from_c_float(nj_state_t *state, double value);
nj_object_t *nj_object_from_c_string(nj_state_t *state, const char *content, int length);
nj_object_t *nj_object_from_c_string_ref(nj_state_t *state, const char *value, size_t length);
nj_object_t *nj_object_from_c_string_ref_2(nj_state_t *state, const char *value, size_t length);
nj_object_t *nj_object_from_c_function(nj_state_t *state, nj_object_t *(*addr)(nj_state_t *state, size_t argc, nj_object_t **argv));

// Operations between Noja values

nj_object_t *nj_object_add(nj_state_t *state, nj_object_t *left, nj_object_t *right);
nj_object_t *nj_object_sub(nj_state_t *state, nj_object_t *left, nj_object_t *right);
nj_object_t *nj_object_mul(nj_state_t *state, nj_object_t *left, nj_object_t *right);
nj_object_t *nj_object_div(nj_state_t *state, nj_object_t *left, nj_object_t *right);
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
nj_object_t *nj_object_shl(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_shr(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_bitwise_and(nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_bitwise_or (nj_state_t *state, nj_object_t *self, nj_object_t *right);
nj_object_t *nj_object_bitwise_xor(nj_state_t *state, nj_object_t *self, nj_object_t *right);

//

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

//

nj_object_t *nj_object_select(nj_state_t *state, nj_object_t *collection, nj_object_t *key);
int 		 nj_object_insert(nj_state_t *state, nj_object_t *collection, nj_object_t *key, nj_object_t *value);

//

nj_object_t *nj_object_type(nj_object_t *self);
nj_object_t *nj_object_test(nj_state_t *state, nj_object_t *object);
nj_object_t *nj_object_print(nj_object_t *state, nj_object_t *object, FILE *fp);
nj_object_t *nj_object_istanciate(nj_state_t *state, nj_object_t *type);

//
nj_object_t *nj_dictionary_merge_in(nj_state_t *state, nj_object_t *dictionary, nj_object_t *other);
nj_object_t *nj_dictionary_select(nj_state_t *state, nj_object_t *dictionary, const char *key);
int 		 nj_dictionary_insert(nj_state_t *state, nj_object_t *dictionary, const char *key, nj_object_t *value);

nj_object_t *nj_array_select(nj_state_t *state, nj_object_t *array, int64_t key);
int 		 nj_array_insert(nj_state_t *state, nj_object_t *array, int64_t key, nj_object_t *value);

