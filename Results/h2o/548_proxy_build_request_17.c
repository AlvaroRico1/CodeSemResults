static void build_request(h2o_req_t *req, h2o_iovec_t *method, h2o_url_t *url, h2o_headers_t *headers,
                          h2o_httpclient_properties_t *props, int keepalive, const char *upgrade_to, int use_proxy_protocol,
                          int *reprocess_if_too_early, h2o_url_t *origin)
{
    size_t remote_addr_len = SIZE_MAX;
    char remote_addr[NI_MAXHOST];
    struct sockaddr_storage ss;
    socklen_t sslen;
    h2o_iovec_t cookie_buf = {NULL}, xff_buf = {NULL}, via_buf = {NULL};
    int preserve_x_forwarded_proto = req->conn->ctx->globalconf->proxy.preserve_x_forwarded_proto;
    int emit_x_forwarded_headers = req->conn->ctx->globalconf->proxy.emit_x_forwarded_headers;
    int emit_via_header = req->conn->ctx->globalconf->proxy.emit_via_header;

    /* for x-f-f */
    if ((sslen = req->conn->callbacks->get_peername(req->conn, (void *)&ss)) != 0)
        remote_addr_len = h2o_socket_getnumerichost((void *)&ss, sslen, remote_addr);

    if (props->proxy_protocol != NULL && use_proxy_protocol) {
        props->proxy_protocol->base = h2o_mem_alloc_pool(&req->pool, char, H2O_PROXY_HEADER_MAX_LENGTH);
        props->proxy_protocol->len = h2o_stringify_proxy_header(req->conn, props->proxy_protocol->base);
    }

    /* method */
    *method = h2o_strdup(&req->pool, req->method.base, req->method.len);

    /* url */
    h2o_url_init(url, origin->scheme, req->authority, h2o_strdup(&req->pool, req->path.base, req->path.len));

    if (props->connection_header != NULL) {
        if (upgrade_to != NULL && upgrade_to != h2o_httpclient_upgrade_to_connect) {
            *props->connection_header = h2o_iovec_init(H2O_STRLIT("upgrade"));
            h2o_add_header(&req->pool, headers, H2O_TOKEN_UPGRADE, NULL, upgrade_to, strlen(upgrade_to));
        } else if (keepalive) {
            *props->connection_header = h2o_iovec_init(H2O_STRLIT("keep-alive"));
        } else {
            *props->connection_header = h2o_iovec_init(H2O_STRLIT("close"));
        }
    }

    /* setup CL or TE, if necessary; chunked encoding is used when the request body is stream and content-length is unknown */
    if (!req->is_tunnel_req) {
        if (req->proceed_req == NULL) {
            if (req->entity.base != NULL || req_requires_content_length(req)) {
                h2o_iovec_t cl_buf = build_content_length(&req->pool, req->entity.len);
                h2o_add_header(&req->pool, headers, H2O_TOKEN_CONTENT_LENGTH, NULL, cl_buf.base, cl_buf.len);
            }
        } else {
            if (req->content_length != SIZE_MAX) {
                h2o_iovec_t cl_buf = build_content_length(&req->pool, req->content_length);
                h2o_add_header(&req->pool, headers, H2O_TOKEN_CONTENT_LENGTH, NULL, cl_buf.base, cl_buf.len);
            } else if (props->chunked != NULL) {
                *props->chunked = 1;
                h2o_add_header(&req->pool, headers, H2O_TOKEN_TRANSFER_ENCODING, NULL, H2O_STRLIT("chunked"));
            }
        }
    }

    /* headers */
    {
        const h2o_header_t *h, *h_end;
        int found_early_data = 0;
        for (h = req->headers.entries, h_end = h + req->headers.size; h != h_end; ++h) {
            if (h2o_iovec_is_token(h->name)) {
                const h2o_token_t *token = (void *)h->name;
                if (token->flags.proxy_should_drop_for_req)
                    continue;
                if (token == H2O_TOKEN_COOKIE) {
                    /* merge the cookie headers; see HTTP/2 8.1.2.5 and HTTP/1 (RFC6265 5.4) */
                    /* FIXME current algorithm is O(n^2) against the number of cookie headers */
                    cookie_buf = build_request_merge_headers(&req->pool, cookie_buf, h->value, ';');
                    continue;
                } else if (token == H2O_TOKEN_VIA) {
                    if (!emit_via_header) {
                        goto AddHeader;
                    }
                    via_buf = build_request_merge_headers(&req->pool, via_buf, h->value, ',');
                    continue;
                } else if (token == H2O_TOKEN_X_FORWARDED_FOR) {
                    if (!emit_x_forwarded_headers) {
                        goto AddHeader;
                    }
                    xff_buf = build_request_merge_headers(&req->pool, xff_buf, h->value, ',');
                    continue;
                } else if (token == H2O_TOKEN_EARLY_DATA) {
                    found_early_data = 1;
                    goto AddHeader;
                }
            }
            if (!preserve_x_forwarded_proto && h2o_lcstris(h->name->base, h->name->len, H2O_STRLIT("x-forwarded-proto")))
                continue;
        AddHeader:
            if (h2o_iovec_is_token(h->name)) {
                const h2o_token_t *token = (void *)h->name;
                h2o_add_header(&req->pool, headers, token, h->orig_name, h->value.base, h->value.len);
            } else {
                h2o_add_header_by_str(&req->pool, headers, h->name->base, h->name->len, 0, h->orig_name, h->value.base,
                                      h->value.len);
            }


// Source: proxy.c
// Lines 135-232
