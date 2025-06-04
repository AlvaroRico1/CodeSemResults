static void on_head(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_http1client_t *client = sock->data;
    int minor_version, version, http_status, rlen;
    const char *msg;
#define MAX_HEADERS 100
    h2o_header_t *headers;
    h2o_iovec_t *header_names;
    size_t msg_len, num_headers, i;
    h2o_socket_cb reader;

    h2o_timer_unlink(&client->super._timeout);

    if (err != NULL) {
        on_error(client, h2o_httpclient_error_io);
        return;
    }

    client->super._timeout.cb = on_head_timeout;

    headers = h2o_mem_alloc_pool(client->super.pool, *headers, MAX_HEADERS);
    header_names = h2o_mem_alloc_pool(client->super.pool, *header_names, MAX_HEADERS);

    /* continue parsing the responses until we see a final one */
    while (1) {
        /* parse response */
        struct phr_header src_headers[MAX_HEADERS];
        num_headers = MAX_HEADERS;
        rlen = phr_parse_response(sock->input->bytes, sock->input->size, &minor_version, &http_status, &msg, &msg_len, src_headers,
                                  &num_headers, 0);
        switch (rlen) {
        case -1: /* error */
            on_error(client, h2o_httpclient_error_http1_parse_failed);
            return;
        case -2: /* incomplete */
            h2o_timer_link(client->super.ctx->loop, client->super.ctx->io_timeout, &client->super._timeout);
            return;
        }

        client->super.bytes_read.header += rlen;
        client->super.bytes_read.total += rlen;

        version = 0x100 | (minor_version != 0);

        /* fill-in the headers */
        for (i = 0; i != num_headers; ++i) {
            if (src_headers[i].name_len == 0) {
                /* reject multiline header */
                on_error(client, h2o_httpclient_error_http1_line_folding);
                return;
            }
            const h2o_token_t *token;
            char *orig_name = h2o_strdup(client->super.pool, src_headers[i].name, src_headers[i].name_len).base;
            h2o_strtolower((char *)src_headers[i].name, src_headers[i].name_len);
            token = h2o_lookup_token(src_headers[i].name, src_headers[i].name_len);
            if (token != NULL) {
                headers[i].name = (h2o_iovec_t *)&token->buf;
            } else {
                header_names[i] = h2o_iovec_init(src_headers[i].name, src_headers[i].name_len);
                headers[i].name = &header_names[i];
            }
            headers[i].value = h2o_iovec_init(src_headers[i].value, src_headers[i].value_len);
            headers[i].orig_name = orig_name;
            headers[i].flags = (h2o_header_flags_t){0};
        }

        if (!(100 <= http_status && http_status <= 199 && http_status != 101))
            break;

        if (client->super.informational_cb != NULL &&
            client->super.informational_cb(&client->super, version, http_status, h2o_iovec_init(msg, msg_len), headers,
                                           num_headers) != 0) {
            close_client(client);
            return;
        }
        h2o_buffer_consume(&client->sock->input, rlen);
        if (client->sock->input->size == 0) {
            h2o_timer_link(client->super.ctx->loop, client->super.ctx->io_timeout, &client->super._timeout);
            return;
        }
    }

    /* recognize hop-by-hop response headers */
    reader = on_body_until_close;
    if (!h2o_httpclient__tunnel_is_ready(&client->super, http_status)) {
        client->_do_keepalive = minor_version >= 1;
        for (i = 0; i != num_headers; ++i) {
            if (headers[i].name == &H2O_TOKEN_CONNECTION->buf) {
                if (h2o_contains_token(headers[i].value.base, headers[i].value.len, H2O_STRLIT("keep-alive"), ',')) {
                    client->_do_keepalive = 1;
                } else {
                    client->_do_keepalive = 0;
                }
            } else if (headers[i].name == &H2O_TOKEN_TRANSFER_ENCODING->buf) {
                if (h2o_memis(headers[i].value.base, headers[i].value.len, H2O_STRLIT("chunked"))) {
                    /* precond: _body_decoder.chunked is zero-filled */
                    client->_body_decoder.chunked.decoder.consume_trailer = 1;
                    reader = on_body_chunked;
                } else if (h2o_memis(headers[i].value.base, headers[i].value.len, H2O_STRLIT("identity"))) {
                    /* continue */
                } else {
                    on_error(client, h2o_httpclient_error_http1_unexpected_transfer_encoding);
                    return;
                }
            } else if (headers[i].name == &H2O_TOKEN_CONTENT_LENGTH->buf) {
                if ((client->_body_decoder.content_length.bytesleft = h2o_strtosize(headers[i].value.base, headers[i].value.len)) ==
                    SIZE_MAX) {
                    on_error(client, h2o_httpclient_error_invalid_content_length);
                    return;
                }
                if (reader != on_body_chunked)
                    reader = on_body_content_length;
            }
        }
    }

    client->state.res = STREAM_STATE_BODY;
    client->super.timings.response_start_at = h2o_gettimeofday(client->super.ctx->loop);

    /* RFC 2616 4.4 */
    if (client->_method_is_head || http_status == 204 || http_status == 304) {
        client->state.res = STREAM_STATE_CLOSED;
        client->super.timings.response_end_at = h2o_gettimeofday(client->super.ctx->loop);
    } else {
        /* close the connection if impossible to determine the end of the response (RFC 7230 3.3.3) */
        if (reader == on_body_until_close)
            client->_do_keepalive = 0;
    }

    h2o_httpclient_on_head_t on_head = {.version = version,
                                        .status = http_status,
                                        .msg = h2o_iovec_init(msg, msg_len),
                                        .headers = headers,
                                        .num_headers = num_headers,
                                        .header_requires_dup = 1};

    /* call the callback */
    client->super._cb.on_body =
        call_on_head(client, client->state.res == STREAM_STATE_CLOSED ? h2o_httpclient_error_is_eos : NULL, &on_head);

    if (client->state.res == STREAM_STATE_CLOSED) {
        close_response(client);
        return;
    } else if (client->super._cb.on_body == NULL) {
        client->_do_keepalive = 0;
        close_client(client);
        return;
    }

    h2o_buffer_consume(&sock->input, rlen);
    client->_socket_bytes_processed = client->sock->bytes_read - client->sock->input->size;

    client->super._timeout.cb = on_body_timeout;
    h2o_socket_read_start(sock, reader);
    reader(client->sock, 0);

#undef MAX_HEADERS
}


// Source: http1client.c
// Lines 317-474
