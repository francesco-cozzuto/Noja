
#include "ast.h"

int generate(ast_t ast, char **e_data, char **e_code, uint32_t *e_data_size, uint32_t *e_code_size);
int parse(const char *source, int source_length, ast_t *e_ast, string_builder_t *output_builder);

int nj_compile(const char *text, size_t length, char **e_data, char **e_code, uint32_t *e_data_size, uint32_t *e_code_size, string_builder_t *output_builder)
{
	ast_t ast;

	if(!parse(text, length, &ast, output_builder))
		return 0;

	//
	// Generate the bytecode
	//

	if(!generate(ast, e_data, e_code, e_data_size, e_code_size)) {

		string_builder_append(output_builder, "Failed to generate bytecode");
		ast_delete(ast);
		return 0;
	}

	ast_delete(ast);
	return 1;
}