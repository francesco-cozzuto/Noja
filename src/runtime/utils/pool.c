

#include <stdlib.h>
#include "pool.h"

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif


pool_t *pool_create_2(unsigned int chunk_size)
{
    pool_t *pool = malloc(sizeof(pool_t) + sizeof(pool_chunk_t) + chunk_size);

    if(pool == 0)
        return 0;

    pool->chunk = (pool_chunk_t*) (pool + 1);
    pool->chunk->prev = 0;
    pool->size = chunk_size;
    pool->used = 0;

    return pool;
}

pool_t *pool_create()
{
    return pool_create_2(1024);
}

int pool_ensure_space(pool_t *pool, unsigned int min)
{
    if(pool->size < pool->used + min) {

        // append a new chunk
        
        unsigned int size = MAX(min, pool->size);

        pool_chunk_t *chunk = malloc(sizeof(pool_chunk_t) + size);

        if(chunk == 0)
            return 0;

        chunk->prev = pool->chunk;
        pool->chunk = chunk;
        pool->used = 0;
        pool->size = size;
    }

    return 1;
}

void *pool_request(pool_t *pool, unsigned int size)
{
    if(!pool_ensure_space(pool, size))
            return 0;
    
    void *addr = pool->chunk->data + pool->used;

    pool->used += size;

    // align the cursor to 8 bytes

    if(pool->used & 7)
        pool->used = (pool->used & ~7) + 8;

    return addr;
}

void pool_destroy(pool_t *pool)
{
    // free chunks

    pool_chunk_t *chunk = pool->chunk;

    // We don't need to free the first chunk
    // since its allocated alongside the pool_t
    // structure

    while(chunk && chunk->prev) {

        pool_chunk_t *prev = chunk->prev;

        free(chunk);

        chunk = prev;
    }

    // free the pool (and the first chunk)

    free(pool);
}