static void on_close(void *data)
{
    struct on_close_data_t *close_data = data;
    h2o_socketpool_t *pool = close_data->pool;
    __sync_sub_and_fetch(&pool->targets.entries[close_data->target]->_shared.leased_count, 1);
    free(close_data);
    __sync_sub_and_fetch(&pool->_shared.count, 1);
}


// Source: socketpool.c
// Lines 390-397
