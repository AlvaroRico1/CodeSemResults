static size_t selector(h2o_balancer_t *_self, h2o_socketpool_target_vector_t *targets, char *tried)
{
    struct least_conn_t *self = (void *)_self;
    size_t i;
    size_t result_index = -1;
    size_t result_weight = 0;
    size_t result_leased = 1;
    uint64_t leftprod, rightprod;

    assert(targets->size != 0);
    pthread_mutex_lock(&self->mutex);
    for (i = 0; i < targets->size; i++) {
        leftprod = targets->entries[i]->_shared.leased_count;
        leftprod *= result_weight;
        rightprod = result_leased;
        rightprod *= ((unsigned)targets->entries[i]->conf.weight_m1) + 1;
        if (!tried[i] && leftprod < rightprod) {
            result_index = i;
            result_leased = targets->entries[i]->_shared.leased_count;
            result_weight = ((unsigned)targets->entries[i]->conf.weight_m1) + 1;
        }
    }
    pthread_mutex_unlock(&self->mutex);

    assert(result_index < targets->size);
    return result_index;
}


// Source: least_conn.c
// Lines 29-55
