static struct rp_generator_t *proxy_send_prepare(h2o_req_t *req)
{
    struct rp_generator_t *self = h2o_mem_alloc_shared(&req->pool, sizeof(*self), on_generator_dispose);

    self->super.proceed = do_proceed;
    self->super.stop = do_stop;
    self->src_req = req;
    self->client = NULL; /* when connection establish timeouts, self->client remains unset by `h2o_httpclient_connect` */
    self->had_body_error = 0;
    self->up_req.is_head = h2o_memis(req->method.base, req->method.len, H2O_STRLIT("HEAD"));
    self->last_content_before_send = NULL;
    h2o_doublebuffer_init(&self->sending, &h2o_socket_buffer_prototype);
    memset(&req->proxy_stats, 0, sizeof(req->proxy_stats));
    h2o_timer_init(&self->send_headers_timeout, on_send_headers_timeout);
    self->req_done = 0;
    self->res_done = 0;

    return self;
}


// Source: proxy.c
// Lines 672-690
