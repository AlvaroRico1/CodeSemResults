h2o_cache_t *h2o_cache_create(int flags, size_t capacity, uint64_t duration, void (*destroy_cb)(h2o_iovec_t value))
{
    h2o_cache_t *cache = h2o_mem_alloc(sizeof(*cache));

    cache->flags = flags;
    cache->table = kh_init(cache);
    cache->size = 0;
    cache->capacity = capacity;
    h2o_linklist_init_anchor(&cache->lru);
    h2o_linklist_init_anchor(&cache->age);
    cache->duration = duration;
    cache->destroy_cb = destroy_cb;
    if ((cache->flags & H2O_CACHE_FLAG_MULTITHREADED) != 0)
        pthread_mutex_init(&cache->mutex, NULL);

    return cache;
}


// Source: cache.c
// Lines 113-129
