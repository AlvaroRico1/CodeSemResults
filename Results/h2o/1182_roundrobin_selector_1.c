static size_t selector(h2o_balancer_t *balancer, h2o_socketpool_target_vector_t *targets, char *tried)
{
    size_t i;
    size_t result = 0;
    struct round_robin_t *self = (void *)balancer;

    pthread_mutex_lock(&self->mutex);

    assert(targets->size != 0);
    for (i = 0; i < targets->size; i++) {
        if (!tried[self->pos]) {
            /* get the result */
            result = self->pos;
            if (++self->consumed_weight > targets->entries[self->pos]->conf.weight_m1)
                select_next(self, targets);
            pthread_mutex_unlock(&self->mutex);
            return result;
        } else {
            select_next(self, targets);
        }
    }
    h2o_fatal("unreachable");
}


// Source: roundrobin.c
// Lines 39-61
