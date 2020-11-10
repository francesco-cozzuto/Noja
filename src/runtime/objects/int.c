
#include <math.h>
#include "../noja.h"

static void int_print(nj_state_t *state, nj_object_t *self, FILE *fp);

static nj_object_t *int_add(nj_state_t *state, nj_object_t *self, nj_object_t *right);

static nj_object_t *int_sub(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_mul(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_div(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_mod(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_pow(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_lss(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_grt(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_leq(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_geq(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_eql(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_nql(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_and(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_or(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_bitwise_and(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_bitwise_or(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_bitwise_xor(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_shl(nj_state_t *state, nj_object_t *self, nj_object_t *right);
static nj_object_t *int_shr(nj_state_t *state, nj_object_t *self, nj_object_t *right);


static uint8_t int_test(nj_state_t *state, nj_object_t *self);

static void int_print(nj_state_t *state, nj_object_t *self, FILE *fp)
{
	nj_object_int_t *x = (nj_object_int_t*) self;

	(void) state;
	
	fprintf(fp, "%ld", x->value);
}

static nj_object_t *int_add(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in add operation
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value + r->value);
}

static nj_object_t *int_sub(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in sub operation
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value - r->value);
}

static nj_object_t *int_mul(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in mul operation
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value * r->value);
}

static nj_object_t *int_div(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in div operation
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	if(r->value == 0)

		// #ERROR
		// Division by zero
		return 0;

	return nj_object_from_c_int(state, x->value / r->value);
}

static nj_object_t *int_mod(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in mod operation
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value % r->value);
}

static nj_object_t *int_pow(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in pow operation
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, pow(x->value, r->value));
}

static nj_object_t *int_lss(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value < r->value);
}

static nj_object_t *int_grt(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value > r->value);
}

static nj_object_t *int_leq(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value <= r->value);
}

static nj_object_t *int_geq(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value >= r->value);
}

static nj_object_t *int_eql(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return nj_object_from_c_int(state, 0);
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value == r->value);
}

static nj_object_t *int_nql(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value != r->value);
}

static nj_object_t *int_and(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value && r->value);
}

static nj_object_t *int_or(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value || r->value);
}

static nj_object_t *int_bitwise_and(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value & r->value);
}

static nj_object_t *int_bitwise_or(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value | r->value);
}

static nj_object_t *int_bitwise_xor(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value ^ r->value);
}

static nj_object_t *int_shl(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value << r->value);
}

static nj_object_t *int_shr(nj_state_t *state, nj_object_t *self, nj_object_t *right)
{
	(void) state;

	nj_object_int_t *x = (nj_object_int_t*) self;

	if(right->type != (nj_object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	nj_object_int_t *r = (nj_object_int_t*) right;

	return nj_object_from_c_int(state, x->value >> r->value);
}

static uint8_t int_test(nj_state_t *state, nj_object_t *self)
{
	(void) state;
	
	nj_object_int_t *x = (nj_object_int_t*) self;

	return x->value != 0;
}

int int_methods_setup(nj_state_t *state)
{
(void) state;
	/*
	state->type_object_int.methods = nj_object_istanciate(state, (nj_object_t*) &state->type_object_dict);

	assert(state->type_object_int.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		nj_object_t *o = nj_object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!nj_dictionary_insert(state, state->type_object_int.methods, method_names[i], o))
	
			return 0;
	}
	*/

	return 1;
}

int int_setup(nj_state_t *state)
{
	state->type_object_int = (nj_object_type_t) {

		.super = (nj_object_t) { .new_location = 0, .type = (nj_object_t*) &state->type_object_type, .flags = 0 },
		.name = "Int",
		.size = sizeof(nj_object_int_t),
		.methods = 0, // Must be created
		.on_init = 0,
		.on_deinit = 0,
		.on_select = 0,
		.on_insert = 0,
		.on_select_attribute = 0,
		.on_insert_attribute = 0,
		.on_print = int_print,
		.on_add = int_add,
		.on_sub = int_sub,
		.on_mul = int_mul,
		.on_div = int_div,
		.on_mod = int_mod,
		.on_pow = int_pow,
		.on_lss = int_lss,
		.on_grt = int_grt,
		.on_leq = int_leq,
		.on_geq = int_geq,
		.on_eql = int_eql,
		.on_nql = int_nql,
		.on_and = int_and,
		.on_or  = int_or,
		.on_bitwise_and = int_bitwise_and,
		.on_bitwise_or  = int_bitwise_or,
		.on_bitwise_xor = int_bitwise_xor,
		.on_shl = int_shl,
		.on_shr = int_shr,
		.on_test = int_test,
	};

	return 1;
}