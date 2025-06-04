static void *thread_main(void *_ctx)
{
    h2o_memcached_context_t *ctx = _ctx;

    while (1)
        reader_main(ctx);
    return NULL;
}


// Source: memcached.c
// Lines 314-321
