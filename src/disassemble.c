
#include <assert.h>
#include "noja.h"

void disassemble(executable_t *executable, FILE *fp)
{
	char *code, *data;
	int code_length, data_length;

	code = executable->code;
	data = executable->data;
	code_length = executable->code_length;
	data_length = executable->data_length;

	{
		int i = 0;

		while(i < data_length) {

			char c = data[i];

			if(i == 0 || data[i-1] == '\0')
				fprintf(fp, "%-4d | \"", i);

			switch(c) {
				case '\0': fprintf(fp, "\"\n"); break;
				case '\n': fprintf(fp, "\\n"); break;
				case '\t': fprintf(fp, "\\t"); break;
				case '\r': fprintf(fp, "\\s"); break;
				default: fprintf(fp, "%c", c); break;
			}

			i++;
		}
	}

	int i = 0;

	while(i < code_length) {

		uint32_t opcode = *(uint32_t*) (code + i);

		const char *name = get_opcode_name(opcode);

		fprintf(fp, "%-4d | %s ", i, name);

		i += sizeof(uint32_t);

		const char *operands = get_instruction_operands(opcode);

		assert(operands);

		int j = 0;
		while(operands[j]) {

			switch(operands[j]) {
				case 'i':
				{
					fprintf(fp, "%ld", *(int64_t*) (code + i));

					i += sizeof(int64_t);
					break;
				}
				case 's':
				{
					uint32_t offset = *(uint32_t*) (code + i);

					fprintf(fp, "%d (\"%s\")", offset, data + offset);

					i += sizeof(uint32_t);
					break;
				}
				case 'f':
				{
					fprintf(fp, "%f", *(double*) (code + i));

					i += sizeof(double);
					break;
				}

				case 'a':
				{
					fprintf(fp, "%d", *(uint32_t*) (code + i));

					i += sizeof(uint32_t);
					break;
				}

				default: fprintf(fp, "Unexpected operand type [%c]\n", operands[j]);
			}

			if(operands[j+1])
				fprintf(fp, ", ");

			j++;
		}

		fprintf(fp, "\n");
	}
}