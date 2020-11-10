
#include <assert.h>
#include <noja.h>

nj_object_t *exposed_join(nj_state_t *state, size_t argc, nj_object_t *argv)
{
	return 0;
}

nj_object_t *exposed_split(nj_state_t *state, size_t argc, nj_object_t *argv)
{
	return 0;
}

nj_object_t *exposed_extension(nj_state_t *state, size_t argc, nj_object_t *argv)
{
	return 0;
}

nj_object_t *exposed_path(nj_state_t *state, size_t argc, nj_object_t *argv)
{
	return 0;
}

nj_object_t *exposed_name(nj_state_t *state, size_t argc, nj_object_t *argv)
{
	return 0;
}

nj_object_t *setup(nj_state_t *state)
{
	nj_object_t *variables = nj_object_istanciate(state, nj_get_dict_type_object(state));

	assert(variables);

	/*
	{
		nj_object_t *function_object = nj_object_from_c_funtion(state, exposed_load_text);

		if(!nj_dictionart_insert(state, variables, "load_text", function_object))
			return 0;
	}
	*/

}