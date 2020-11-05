
#ifndef _POOL_
#define _POOL_

typedef struct pool_chunk_t pool_chunk_t;
typedef struct pool_t pool_t;

struct pool_chunk_t {
    pool_chunk_t *prev;
    char data[];
};

struct pool_t {
    pool_chunk_t *chunk;
    unsigned int size, used;
};

pool_t *pool_create();
pool_t *pool_create_2(unsigned int chunk_size);
int     pool_ensure_space(pool_t *pool, unsigned int min);
void   *pool_request(pool_t *pool, unsigned int size);
void    pool_destroy(pool_t *pool);

#endif