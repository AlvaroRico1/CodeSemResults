static void destroy(h2o_balancer_t *balancer)
{
    struct round_robin_t *self = (void *)balancer;
    pthread_mutex_destroy(&self->mutex);
    free(self);
}


// Source: roundrobin.c
// Lines 63-68
