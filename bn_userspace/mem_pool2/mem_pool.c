#include <stdio.h>
#include <stdlib.h>

#include "mem_pool.h"

static void *MEMORY;

#define INITIALIZE_MEMORY(size)               \
    do {                                      \
        MEMORY = malloc(sizeof(char) * size); \
    } while (0)

#define INITIALIZE_MEM_CHUNK(ck, mem_start, size) \
    do {                                          \
        ck = malloc(sizeof(_MP_Chunk));           \
        ck->start = mem_start;                    \
        ck->mem_size = size;                      \
        ck->next = NULL;                          \
        ck->is_free = 1;                          \
    } while (0);

#define INITIALIZE_MEM_POOL(p, size)                   \
    do {                                               \
        INITIALIZE_MEMORY(size);                       \
        p = (MemoryPool *) malloc(sizeof(MemoryPool)); \
        p->pool_size = size;                           \
        p->alloc_list = NULL;                          \
        _MP_Chunk *ck;                                 \
        INITIALIZE_MEM_CHUNK(ck, MEMORY, size);        \
        p->free_list = ck;                             \
    } while (0);

#define INSERT_LIST(list, ck)         \
    do {                              \
        _MP_Chunk **indirect = &list; \
        ck->next = *indirect;         \
        *indirect = ck;               \
    } while (0);

void *my_malloc(MemoryPool *p, mem_size_t size)
{
    _MP_Chunk *free_list = p->free_list;

    while (free_list) {
        if (free_list->is_free && free_list->mem_size >= size) {
            _MP_Chunk *allocated_ck;
            INITIALIZE_MEM_CHUNK(allocated_ck, free_list->start, 16);
            allocated_ck->is_free = 0;
            INSERT_LIST(p->alloc_list, allocated_ck);

            free_list->start = (char *) free_list->start + 16;
            free_list->mem_size -= size;
            p->pool_size -= size;

            return allocated_ck->start;
        }

        free_list = free_list->next;
    }

    return (void *) 0;
}



#define alloc_node(n, n_type, size)                                      \
    do {                                                                 \
        n = malloc(sizeof(n_type) * size);                               \
        for (int i = 1; i < size; i++) {                                 \
            ((n_type *) ((char *) n + sizeof(n_type) * (i - 1)))->next = \
                (char *) n + sizeof(n_type) * i;                         \
        }                                                                \
    } while (0);

#define BLOCK_INS_NODE(b, n)                      \
    do {                                          \
        block_t **indirect = &b;                  \
        block_t *new_b = malloc(sizeof(block_t)); \
        new_b->node_list_head = n;                \
        new_b->next = NULL;                       \
        while (*indirect) {                       \
            indirect = &(*indirect)->next;        \
        }                                         \
        *indirect = new_b;                        \
    } while (0);


void show_node_list(node_t *n)
{
    while (n) {
        printf("%d (addr: %p), ", n->val, n);
        n = n->next;
    }
    printf("\n");
}

int main()
{
    printf("Sizeof(node_t): %zu\n", sizeof(node_t));

    node_t *n, *n2;

    alloc_node(n, node_t, 6);
    // show_node_list(n);

    alloc_node(n2, node_t, 4);
    // show_node_list(n2);

    block_t *b = NULL;
    BLOCK_INS_NODE(b, n);
    BLOCK_INS_NODE(b, n2);

    while (b) {
        show_node_list(b->node_list_head);
        b = b->next;
    }

    printf("*****\n");

    // INITIALIZE_MEMORY(16);
    MemoryPool *pool;
    INITIALIZE_MEM_POOL(pool, 16);

    printf("Pool free_list start : %p\n", pool->free_list->start);
    printf("Pool free_list mem size : %llu\n", pool->free_list->mem_size);

    int *a = (int *) my_malloc(pool, sizeof(int));
    printf("Integer a address : %p\n", a);

    printf("* Pool free_list start : %p\n", pool->free_list->start);
    printf("* Pool free_list mem size : %llu\n", pool->free_list->mem_size);

    return 0;
}