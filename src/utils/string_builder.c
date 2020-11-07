
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "string_builder.h"

void string_builder_init(string_builder_t *builder)
{
	builder->tail = &builder->head;
	builder->head.next = 0;
	builder->tail_used = 0;
	builder->length = 0;
}

void string_builder_deinit(string_builder_t *builder)
{
	string_builder_chunk_t *chunk = builder->head.next;

	while(chunk) {

		string_builder_chunk_t *next_chunk = chunk->next;

		free(chunk);

		chunk = next_chunk;
	}
}

int string_builder_append_byte(string_builder_t *builder, char c)
{
	if(builder->tail_used == BYTES_PER_CHUNK) {

		builder->tail->content[builder->tail_used] = '\0';

		string_builder_chunk_t *chunk = malloc(sizeof(string_builder_chunk_t));

		if(chunk == 0)
			return 0;

		chunk->next = 0;
		builder->tail->next = chunk;
		builder->tail = chunk;
		builder->tail_used = 0;
	}

	builder->tail->content[builder->tail_used++] = c;
	builder->length++;
	return 1;
}

static int string_builder_append_plain_string(string_builder_t *builder, char *string, int length)
{

	if(length < 0)
		length = strlen(string);

	for(int i = 0; i < length; i++)
		if(!string_builder_append_byte(builder, string[i]))
			return 0;
	return 1;
}

static int string_builder_append_int(string_builder_t *builder, int v)
{
	char buffer[1024];

	sprintf(buffer, "%d", v);

	return string_builder_append_plain_string(builder, buffer, -1);
}

static int string_builder_append_double(string_builder_t *builder, double v)
{
	char buffer[1024];

	sprintf(buffer, "%g", v);

	return string_builder_append_plain_string(builder, buffer, -1);
}

int string_builder_append(string_builder_t *builder, const char *fmt, ...) 
{
	va_list args;
	va_start(args, fmt);

	int result = string_builder_append_p(builder, fmt, args);

	va_end(args);
	return result;
}

int string_builder_append_p(string_builder_t *builder, const char *fmt, va_list args)
{
	char c;
	int i = 0;

	while(1) {

		c = fmt[i++];

		if(c == '\0')
			break;

		if(c == '$') {

			c = fmt[i++];

			if(c == '{') {

				char keyword[128];
				int keyword_length = 0;

				while(((c = fmt[i++]) != '}') && c != '\0')
					if(keyword_length < 128-1 && c != ' ' && c != '\t' && c != '\n')
						keyword[keyword_length++] = c;
				keyword[keyword_length] = '\0';

				if(!strcmp(keyword, "integer")) {

					int v = va_arg(args, int);

					if(!string_builder_append_int(builder, v))
						return 0;

				} else if(!strcmp(keyword, "floating")) {

					double v = va_arg(args, double);

					if(!string_builder_append_double(builder, v))
						return 0;

				} else if(!strcmp(keyword, "string-with-length")) {

					char *v = va_arg(args, char*);
					int length = va_arg(args, int);

					if(!string_builder_append_plain_string(builder, v, length))
						return 0;

				} else if(!strcmp(keyword, "zero-terminated-string")) {

					char *v = va_arg(args, char*);

					if(!string_builder_append_plain_string(builder, v, -1))
						return 0;

				} else {

					if(!string_builder_append_plain_string(builder, "<Unknown placeholder>", -1))
						return 0;
				}
			
			} else {

				if(!string_builder_append_byte(builder, '$'))
					return 0;

				i--;
			}

		} else {

			if(!string_builder_append_byte(builder, c))
				return 0;
		}
	}

	return 1;
}

void string_builder_serialize_to_buffer(string_builder_t *builder, char *dest)
{
	builder->tail->content[builder->tail_used] = '\0';

	string_builder_chunk_t *chunk = &builder->head;

	size_t written = 0;

	while(chunk) {

		strcpy(dest + written, chunk->content);

		if(!chunk->next) {
			
			written += builder->tail_used;

		} else {
		
			written += BYTES_PER_CHUNK;
		}

		chunk = chunk->next;
	}

	dest[written] = '\0';
}

void string_builder_serialize_to_stream(string_builder_t *builder, FILE *fp)
{
	builder->tail->content[builder->tail_used] = '\0';

	string_builder_chunk_t *chunk = &builder->head;

	while(chunk) {

		fprintf(fp, "%s", chunk->content);

		chunk = chunk->next;
	}
}
