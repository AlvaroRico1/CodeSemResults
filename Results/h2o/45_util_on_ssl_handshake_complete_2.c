static void on_ssl_handshake_complete(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_accept_data_t *data = sock->data;
    sock->data = NULL;

    if (err != NULL) {
        ++data->ctx->ctx->ssl.errors;
        h2o_socket_close(sock);
        goto Exit;
    }

    /* stats for handshake */
    struct timeval handshake_completed_at = h2o_gettimeofday(data->ctx->ctx->loop);
    int64_t handshake_time = h2o_timeval_subtract(&data->connected_at, &handshake_completed_at);
    if (h2o_socket_get_ssl_session_reused(sock)) {
        ++data->ctx->ctx->ssl.handshake_resume;
        data->ctx->ctx->ssl.handshake_accum_time_resume += handshake_time;
    } else {
        ++data->ctx->ctx->ssl.handshake_full;
        data->ctx->ctx->ssl.handshake_accum_time_full += handshake_time;
    }

    h2o_iovec_t proto = h2o_socket_ssl_get_selected_protocol(sock);
    const h2o_iovec_t *ident;
    for (ident = h2o_http2_alpn_protocols; ident->len != 0; ++ident) {
        if (proto.len == ident->len && memcmp(proto.base, ident->base, proto.len) == 0) {
            /* connect as http2 */
            ++data->ctx->ctx->ssl.alpn_h2;
            h2o_http2_accept(data->ctx, sock, data->connected_at);
            goto Exit;
        }
    }
    /* connect as http1 */
    if (proto.len != 0)
        ++data->ctx->ctx->ssl.alpn_h1;
    h2o_http1_accept(data->ctx, sock, data->connected_at);

Exit:
    accept_data_callbacks.destroy(data);
}


// Source: util.c
// Lines 340-379
