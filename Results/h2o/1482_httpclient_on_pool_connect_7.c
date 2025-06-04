static void on_pool_connect(h2o_socket_t *sock, const char *errstr, void *data, h2o_url_t *origin)
{
    h2o_httpclient_t *client = data;

    h2o_timer_unlink(&client->_timeout);

    client->_connect_req = NULL;

    if (sock == NULL) {
        assert(errstr != NULL);
        on_connect_error(client, errstr);
        return;
    }

    h2o_iovec_t alpn_proto;
    if (client->ctx->force_cleartext_http2 && sock->ssl == NULL && client->ctx->protocol_selector.ratio.http2 == 100) {
        /* The client has prior knowledge and wants to use http2 without
         * discovery or upgrade. Make sure that we're using a 100%
         * H2 context, to avoid having this connection reused for H1
         * accidentaly */
        goto ForceH2;
    } else if (sock->ssl == NULL || (alpn_proto = h2o_socket_ssl_get_selected_protocol(sock)).len == 0) {
        h2o_httpclient__h1_on_connect(client, sock, origin);
    } else {
        if (h2o_memis(alpn_proto.base, alpn_proto.len, H2O_STRLIT("h2"))) {
        ForceH2:
            /* detach this socket from the socketpool to count the number of h1 connections correctly */
            h2o_socketpool_detach(client->connpool->socketpool, sock);
            h2o_httpclient__h2_on_connect(client, sock, origin);
        } else if (memcmp(alpn_proto.base, "http/1.1", alpn_proto.len) == 0) {
            h2o_httpclient__h1_on_connect(client, sock, origin);
        } else {
            on_connect_error(client, h2o_httpclient_error_unknown_alpn_protocol);
        }
    }
}


// Source: httpclient.c
// Lines 110-145
