
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

void report(char *error_buffer, int error_buffer_size, const char *format, ...) {

	if(!error_buffer || !error_buffer_size)
		return;

	va_list args;
	va_start(args, format);

	int i = 0;
	int error_buffer_used = 0;
	int escaped = 0;

	while(1) {
		
		char c = format[i++];

		if(c == '\0')
			break;

		if(c != '%') {

			if(!escaped) {

				if(c == '\\') {

					escaped = 1;

				} else {

					if(error_buffer_used + 1 == error_buffer_size)
						break;

					error_buffer[error_buffer_used++] = c;
				}

			} else {

				switch(c) {
					case '\\': c = '\\'; break;
					case 'n': c = '\n'; break;
					case 't': c = '\t'; break;
					default: assert(0);
				}

				if(error_buffer_used + 1 == error_buffer_size)
					break;

				error_buffer[error_buffer_used++] = c;

				escaped = 0;
			}


		} else {

			c = format[i++];

			switch(c) {
				case 's':
				{
					const char *value = va_arg(args, const char*);
					int length = va_arg(args, int);

					if(length < 0)
						length = strlen(value);

					for(int j = 0; j < length; j++) {

						if(error_buffer_used + 1 == error_buffer_size)
							break;

						error_buffer[error_buffer_used++] = value[j];
					}
					break;
				}

				case 'd':
				{
					int value = va_arg(args, int);

					char temp[16];
					int  temp_length;

					sprintf(temp, "%d", value);

					temp_length = strlen(temp);

					for(int j = 0; j < temp_length; j++) {

						if(error_buffer_used + 1 == error_buffer_size)
							break;

						error_buffer[error_buffer_used++] = temp[j];
					}

					break;
				}

				default:
				fprintf(stderr, ">> Unsupported placeholder \"%%%c\" in \"%s\" at offset %d\n", c, format, i-2);
				assert(0);
			}
		}

	}

	va_end(args);

	error_buffer[error_buffer_used] = '\0';
}

int get_lineno_of_offset(const char *text, int offset) {

	int i = 0;
	int lineno = 1;

	while(i < offset) {

		if(text[i] == '\n')
			lineno++;

		i++;
	}

	return lineno;
}