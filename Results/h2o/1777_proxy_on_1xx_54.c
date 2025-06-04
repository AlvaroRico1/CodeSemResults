static int on_1xx(h2o_httpclient_t *client, int version, int status, h2o_iovec_t msg, h2o_header_t *headers, size_t num_headers)
{
    struct rp_generator_t *self = client->data;
    size_t i;

    for (i = 0; i != num_headers; ++i) {
        if (headers[i].name == &H2O_TOKEN_LINK->buf)
            h2o_push_path_in_link_header(self->src_req, headers[i].value.base, headers[i].value.len);
    }

    if (status != 101) {
        self->src_req->res.status = status;
        self->src_req->res.headers = (h2o_headers_t){headers, num_headers, num_headers};
        h2o_send_informational(self->src_req);
    }

    return 0;
}


// Source: proxy.c
// Lines 542-559
