static void on_setup_ostream(h2o_filter_t *_self, h2o_req_t *req, h2o_ostream_t **slot)
{
    struct st_server_timing_filter_t *self = (struct st_server_timing_filter_t *)_self;

    /* indicate the protocol handler to emit server timing header (basic and proxy properties) */
    req->send_server_timing = H2O_SEND_SERVER_TIMING_BASIC | H2O_SEND_SERVER_TIMING_PROXY;

    /* force chunked encoding for HTTP/1.1 if enforce flag is set */
    if (0x101 <= req->version && req->version < 0x200 && self->enforce)
        req->res.content_length = SIZE_MAX;

    h2o_setup_next_ostream(req, slot);
}


// Source: server_timing.c
// Lines 33-45
