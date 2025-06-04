static void on_filter_setup_ostream(h2o_filter_t *_self, h2o_req_t *req, h2o_ostream_t **slot)
{
    struct st_errordoc_filter_t *self = (void *)_self;
    h2o_errordoc_t *errordoc;
    struct st_errordoc_prefilter_t *prefilter;
    h2o_iovec_t method;
    h2o_ostream_t *ostream;
    size_t i;

    if (req->res.status >= 400 && !prefilter_is_registered(req)) {
        size_t i;
        for (i = 0; i != self->errordocs.size; ++i) {
            errordoc = self->errordocs.entries + i;
            if (errordoc->status == req->res.status)
                goto Found;
        }
    }

    /* bypass to the next filter */
    h2o_setup_next_ostream(req, slot);
    return;

Found:
    /* register prefilter that rewrites the status code after the internal redirect is processed */
    prefilter = (void *)h2o_add_prefilter(req, H2O_ALIGNOF(*prefilter), sizeof(*prefilter));
    prefilter->super.on_setup_ostream = on_prefilter_setup_stream;
    prefilter->req_headers = req->headers;
    prefilter->status = req->res.status;
    prefilter->reason = req->res.reason;
    prefilter->res_headers = (h2o_headers_t){NULL};
    for (i = 0; i != req->res.headers.size; ++i) {
        const h2o_header_t *header = req->res.headers.entries + i;
        if (!(header->name == &H2O_TOKEN_CONTENT_TYPE->buf || header->name == &H2O_TOKEN_CONTENT_LANGUAGE->buf))
            add_header(&req->pool, &prefilter->res_headers, header);
    }
    /* redirect internally to the error document */
    method = req->method;
    if (h2o_memis(method.base, method.len, H2O_STRLIT("POST")))
        method = h2o_iovec_init(H2O_STRLIT("GET"));
    req->headers = (h2o_headers_t){NULL};
    req->res.headers = (h2o_headers_t){NULL};
    h2o_send_redirect_internal(req, method, errordoc->url.base, errordoc->url.len, 0);
    /* create fake ostream that swallows the contents emitted by the generator */
    ostream = h2o_add_ostream(req, H2O_ALIGNOF(*ostream), sizeof(*ostream), slot);
    ostream->do_send = on_ostream_send;
}


// Source: errordoc.c
// Lines 84-129
