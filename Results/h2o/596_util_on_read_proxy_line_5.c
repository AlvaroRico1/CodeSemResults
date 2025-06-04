static void on_read_proxy_line(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_accept_data_t *data = sock->data;

    if (err != NULL) {
        accept_data_callbacks.destroy(data);
        h2o_socket_close(sock);
        return;
    }

    struct sockaddr_storage addr;
    socklen_t addrlen;
    ssize_t r = parse_proxy_line(sock->input->bytes, sock->input->size, (void *)&addr, &addrlen);
    switch (r) {
    case -1: /* error, just pass the input to the next handler */
        break;
    case -2: /* incomplete */
        return;
    default:
        h2o_buffer_consume(&sock->input, r);
        if (addrlen != 0)
            h2o_socket_setpeername(sock, (void *)&addr, addrlen);
        break;
    }

    if (data->ctx->ssl_ctx != NULL) {
        h2o_socket_ssl_handshake(sock, data->ctx->ssl_ctx, NULL, h2o_iovec_init(NULL, 0), on_ssl_handshake_complete);
    } else {
        struct st_h2o_accept_data_t *data = sock->data;
        sock->data = NULL;
        h2o_http1_accept(data->ctx, sock, data->connected_at);
        accept_data_callbacks.destroy(data);
    }


// Source: util.c
// Lines 477-509
