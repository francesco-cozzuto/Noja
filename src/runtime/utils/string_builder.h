
#include <stdarg.h>
#include <stdio.h>

#define BYTES_PER_CHUNK 4096-1

typedef struct string_builder_chunk_t string_builder_chunk_t;
struct string_builder_chunk_t {
	string_builder_chunk_t *next;
	char content[BYTES_PER_CHUNK+1];
};

typedef struct string_builder_t string_builder_t;
struct string_builder_t {
	string_builder_chunk_t head, *tail;
	size_t tail_used;
	size_t length;
};

void  string_builder_init(string_builder_t *builder);
void  string_builder_deinit(string_builder_t *builder);
int   string_builder_append(string_builder_t *builder, const char *fmt, ...);
int   string_builder_append_p(string_builder_t *builder, const char *fmt, va_list args);
int   string_builder_append_byte(string_builder_t *builder, char c);
void  string_builder_serialize_to_buffer(string_builder_t *builder, char *dest);
void  string_builder_serialize_to_stream(string_builder_t *builder, FILE *fp);
