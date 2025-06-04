static h2o_httpclient_t *detach_client(struct rp_generator_t *self)
{
    h2o_httpclient_t *client = self->client;
    assert(client != NULL);
    client->data = NULL;
    self->client = NULL;
    return client;
}


// Source: proxy.c
// Lines 281-288
