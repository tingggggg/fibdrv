#define mem_size_t unsigned long long

typedef struct _mp_chunk {
    void *start;
    mem_size_t mem_size;
    struct _mp_chunk *next;
    int is_free;
} _MP_Chunk;

typedef struct _mp_mem_pool {
    mem_size_t pool_size;
    _MP_Chunk *free_list;
    _MP_Chunk *alloc_list;
} MemoryPool;

void *my_malloc(MemoryPool *p, mem_size_t size);
void my_free(MemoryPool *pool, void *s);