static void proceed_request(h2o_httpclient_t *client, const char *errstr)
{
    struct rp_generator_t *self = client->data;
    if (self == NULL)
        return;
    if (errstr != NULL)
        detach_client(self);
    if (self->src_req->proceed_req != NULL)
        self->src_req->proceed_req(self->src_req, errstr);
}


// Source: proxy.c
// Lines 561-570
