static void do_proceed(h2o_generator_t *generator, h2o_req_t *req)
{
    struct rp_generator_t *self = (void *)generator;

    h2o_doublebuffer_consume(&self->sending);
    do_send(self);
    if (self->last_content_before_send == NULL)
        self->client->update_window(self->client);
}


// Source: proxy.c
// Lines 342-350
