static void do_stop(h2o_generator_t *generator, h2o_req_t *req)
{
    struct rp_generator_t *self = (void *)generator;
    do_close(self);
}


// Source: proxy.c
// Lines 309-313
