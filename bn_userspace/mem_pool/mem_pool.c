#include "mem_pool.h"

#define MP_CHUNKHEADER sizeof(struct _mp_memory_chunk)
#define MP_CHUNKEND sizeof(struct _mp_memory_chunk *)

#define ALIGN_SIZE(_n, align_t)                                \
    (_n + (sizeof(align_t) * !!((sizeof(align_t) - 1) & _n)) - \
     ((sizeof(align_t) - 1) & _n))

#define MP_INIT_MEMORY_LIST(ml, mem_size)           \
    do {                                            \
        ml->mem_pool_size = mem_size;               \
        ml->alloc_mem = 0;                          \
        ml->alloc_prog_mem = 0;                     \
        ml->free_list = (_MemoryChunk *) ml->start; \
        ml->free_list->is_free = 1;                 \
        ml->free_list->alloc_mem = mem_size;        \
        ml->free_list->prev = NULL;                 \
        ml->free_list->next = NULL;                 \
        ml->alloc_list = NULL;                      \
    } while (0)

#define MP_DLINKLIST_INT_FRT(head, x) \
    do {                              \
        x->prev = NULL;               \
        x->next = head;               \
        if (head)                     \
            head->prev = x;           \
        head = x;                     \
    } while (0)

#define MP_DLINKLIST_DEL(head, x)        \
    do {                                 \
        if (!x->prev) {                  \
            head = x->next;              \
            if (x->next)                 \
                x->next->prev = NULL;    \
        } else {                         \
            x->prev->next = x->next;     \
            if (x->next)                 \
                x->next->prev = x->prev; \
        }                                \
    } while (0)

MemoryPool *MemoryPoolInit(mem_size_t maxMemPoolSize, mem_size_t memPoolSize)
{
    if (memPoolSize > maxMemPoolSize) {
        return NULL;
    }

    MemoryPool *mp = (MemoryPool *) malloc(sizeof(MemoryPool));
    if (!mp)
        return NULL;

    mp->last_id = 0;
    if (memPoolSize < maxMemPoolSize)
        mp->auto_extend = 1;
    mp->max_mem_pool_size = maxMemPoolSize;
    mp->total_mem_pool_size = mp->mem_pool_size = memPoolSize;

    char *s =
        (char *) malloc(sizeof(_MemoryList) + sizeof(char) * mp->mem_pool_size);
    if (!s) {
        free(mp);
        mp = NULL;
        return NULL;
    }


    mp->mlist = (_MemoryList *) s;
    mp->mlist->start = s + sizeof(_MemoryList);
    MP_INIT_MEMORY_LIST(mp->mlist, mp->mem_pool_size);
    mp->mlist->next = NULL;
    mp->mlist->id = mp->last_id++;

    return mp;
}

void *MemoryPoolAlloc(MemoryPool *mp, mem_size_t allocSize)
{
    if (allocSize == 0)
        return NULL;

    mem_size_t total_needed_size =
        ALIGN_SIZE(allocSize + MP_CHUNKHEADER + MP_CHUNKEND, long);
    if (total_needed_size > mp->mem_pool_size)
        return NULL;
    // printf("total_needed_size: %d\n", total_needed_size);

    _MemoryList *ml = NULL;
    _MemoryChunk *_free = NULL, *_not_free = NULL;

    // FIND_FREE_CHUNK:
    ml = mp->mlist;
    while (ml) {
        if (mp->mem_pool_size - ml->alloc_mem < total_needed_size) {
            ml = ml->next;
            continue;
        }

        _free = ml->free_list;
        // printf("first _free : %ld\n", _free);
        _not_free = NULL;
        while (_free) {
            if (_free->alloc_mem >= total_needed_size) {
                if (_free->alloc_mem - total_needed_size >
                    (MP_CHUNKHEADER + MP_CHUNKEND)) {
                    _not_free = _free;

                    _free = (_MemoryChunk *) ((char *) _not_free +
                                              total_needed_size);
                    *_free = *_not_free;  // copy origin free chunk status to
                                          // new free chunk
                    _free->alloc_mem -=
                        total_needed_size;  // reduce availabel space
                    *(_MemoryChunk **) ((char *) _free + _free->alloc_mem -
                                        MP_CHUNKEND) = _free;

                    // printf("address of _free: %ld\n", _free);
                    // printf("address of .. : %ld\n", (char*) _free +
                    // _free->alloc_mem); printf("address of .. - MP_CHUNKEND:
                    // %ld\n", (char*) _free + _free->alloc_mem - MP_CHUNKEND);

                    if (!_free->prev) {
                        ml->free_list = _free;
                    } else {
                        _free->prev->next = _free;
                    }
                    if (_free->next)
                        _free->next->prev = _free;

                    _not_free->is_free = 0;
                    _not_free->alloc_mem = total_needed_size;

                    *(_MemoryChunk **) ((char *) _not_free + total_needed_size -
                                        MP_CHUNKEND) = _not_free;
                } else {
                    _not_free = _free;
                    MP_DLINKLIST_DEL(ml->free_list, _not_free);
                    _not_free->is_free = 0;
                }
                MP_DLINKLIST_INT_FRT(ml->alloc_list, _not_free);

                ml->alloc_mem += _not_free->alloc_mem;
                ml->alloc_prog_mem +=
                    (_not_free->alloc_mem - MP_CHUNKHEADER - MP_CHUNKEND);

                // printf("((char*) _not_free = MP_CHUNKHEADER): %ld\n",
                // ((char*) _not_free + MP_CHUNKHEADER)); printf("_free->prev
                // :%ld\n", _free->prev);
                return (void *) ((char *) _not_free + MP_CHUNKHEADER);
            }
            _free = _free->next;
        }

        ml = ml->next;
    }

    // err_out:
    return NULL;
}
