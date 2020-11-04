
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


#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

char is_whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\r';
}

int read_line(FILE *src, char **e_buffer, int *e_length)
{ 

	char *buffer = malloc(65536);

	int buffer_size = 65536;
	int buffer_used = 0;

	char c;

	while((c = getc(src)) != '\n') {

		// Append the new character

		buffer[buffer_used++] = c;


		// Resize the buffer if needed

		if(buffer_size == buffer_used+1) {


			buffer = realloc(buffer, buffer_size * 2);


			if(buffer == 0) {

				// Release and quit!

				free(buffer);
				return 0;
			}

			buffer_size *= 2;
		}	
	}

	// Terminate the buffer with a zero byte

	buffer[buffer_used] = '\0';

	// Output the results

	*e_buffer = buffer;
	*e_length = buffer_used;

	return 1;
}

void skip_string(char *buffer, int *i)
{

	char c, escaped = 0;

	while(1) {

		c = buffer[(*i)++];

		if(escaped) {

			escaped = 0;

		} else {

			if(c == '"' || c == '\0')

				break;

			if(c == '\\')

				escaped = 1;
		}
	}

	if(c != '\0')
		(*i)--;
}

void skip_non_whitespace(char *buffer, int *i)
{

	char c, escaped = 0;

	while(1) {

		c = buffer[(*i)++];

		if(escaped) {

			escaped = 0;

		} else {

			if(c == '\\')
				escaped = 1;

			if(is_whitespace(c) || c == '\0')
				break;
		}
	}

	(*i)--;
}

int tokenize_buffer(char *buffer, char ***e_tokens, int *e_token_count)
{
	char **tokens = malloc(sizeof(char*) * 8);
	int token_count = 0, token_allocated_count = 8;

	int i = 0;
	char c;

	while(1) {

		while(is_whitespace(c = buffer[i++]));

		if(c == '\0')
			break;

		// Resize the token array if needed

		if(token_count == token_allocated_count-1) {

			tokens = realloc(tokens, sizeof(char*) * token_allocated_count * 2);

			if(tokens == 0)
				return 0;

			token_allocated_count *= 2;
		}


		tokens[token_count++] = buffer + i - 1;


		// Skip until the end of the token

		if(c == '\"') {

			skip_string(buffer, &i);

		} else {

			skip_non_whitespace(buffer, &i);
 		}


 		// Inject a null byte into the buffer

 		if(buffer[i] == '\0')
 			break;

 		buffer[i] = '\0';

 		i++;

	}

	tokens[token_count] = 0;

	*e_tokens = tokens;
	*e_token_count = token_count;

	return 1;
}