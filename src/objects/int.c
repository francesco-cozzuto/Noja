
#include <math.h>
#include "../noja.h"

static void int_print(state_t *state, object_t *self, FILE *fp);

static object_t *int_add(state_t *state, object_t *self, object_t *right);

static object_t *int_sub(state_t *state, object_t *self, object_t *right);
static object_t *int_mul(state_t *state, object_t *self, object_t *right);
static object_t *int_div(state_t *state, object_t *self, object_t *right);
static object_t *int_mod(state_t *state, object_t *self, object_t *right);
static object_t *int_pow(state_t *state, object_t *self, object_t *right);
static object_t *int_lss(state_t *state, object_t *self, object_t *right);
static object_t *int_grt(state_t *state, object_t *self, object_t *right);
static object_t *int_leq(state_t *state, object_t *self, object_t *right);
static object_t *int_geq(state_t *state, object_t *self, object_t *right);
static object_t *int_eql(state_t *state, object_t *self, object_t *right);
static object_t *int_nql(state_t *state, object_t *self, object_t *right);
static object_t *int_and(state_t *state, object_t *self, object_t *right);
static object_t *int_or(state_t *state, object_t *self, object_t *right);
static object_t *int_bitwise_and(state_t *state, object_t *self, object_t *right);
static object_t *int_bitwise_or(state_t *state, object_t *self, object_t *right);
static object_t *int_bitwise_xor(state_t *state, object_t *self, object_t *right);
static object_t *int_shl(state_t *state, object_t *self, object_t *right);
static object_t *int_shr(state_t *state, object_t *self, object_t *right);


static uint8_t int_test(state_t *state, object_t *self);

static void int_print(state_t *state, object_t *self, FILE *fp)
{
	object_int_t *x = (object_int_t*) self;

	(void) state;
	
	fprintf(fp, "%ld", x->value);
}

static object_t *int_add(state_t *state, object_t *self, object_t *right)
{
	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in add operation
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value + r->value);
}

static object_t *int_sub(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in sub operation
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value - r->value);
}

static object_t *int_mul(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in mul operation
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value * r->value);
}

static object_t *int_div(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in div operation
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	if(r->value == 0)

		// #ERROR
		// Division by zero
		return 0;

	return object_from_cint(state, x->value / r->value);
}

static object_t *int_mod(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in mod operation
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value % r->value);
}

static object_t *int_pow(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type in pow operation
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, pow(x->value, r->value));
}

static object_t *int_lss(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value < r->value);
}

static object_t *int_grt(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value > r->value);
}

static object_t *int_leq(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value <= r->value);
}

static object_t *int_geq(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value >= r->value);
}

static object_t *int_eql(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value == r->value);
}

static object_t *int_nql(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value != r->value);
}

static object_t *int_and(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value && r->value);
}

static object_t *int_or(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value || r->value);
}

static object_t *int_bitwise_and(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value & r->value);
}

static object_t *int_bitwise_or(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value | r->value);
}

static object_t *int_bitwise_xor(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value ^ r->value);
}

static object_t *int_shl(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value << r->value);
}

static object_t *int_shr(state_t *state, object_t *self, object_t *right)
{
	(void) state;

	object_int_t *x = (object_int_t*) self;

	if(right->type != (object_t*) &state->type_object_int) {

		// #ERROR
		// Unexpected type
		return 0;
	}

	object_int_t *r = (object_int_t*) right;

	return object_from_cint(state, x->value >> r->value);
}

static uint8_t int_test(state_t *state, object_t *self)
{
	(void) state;
	
	object_int_t *x = (object_int_t*) self;

	return x->value != 0;
}

int int_methods_setup(state_t *state)
{
	state->type_object_int.methods = object_istanciate(state, (object_t*) &state->type_object_dict);

	assert(state->type_object_int.methods);

	static const char *method_names[] = {};
	static const builtin_interface_t method_routines[] = {};

	for(size_t i = 0; i < sizeof(method_names) / sizeof(char*); i++) {

		object_t *o = object_from_cfunction(state, method_routines[i]);

		if(o == 0)
			return 0;

		if(!dict_cinsert(state, state->type_object_int.methods, method_names[i], o))
	
			return 0;
	}

	return 1;
}

int int_setup(state_t *state)
{
	state->type_object_int = (object_type_t) {

		.super = (object_t) { .new_location = 0, .type = (object_t*) &state->type_object_type, .flags = 0 },
		.name = "Int",
		.size = sizeof(object_int_t),
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