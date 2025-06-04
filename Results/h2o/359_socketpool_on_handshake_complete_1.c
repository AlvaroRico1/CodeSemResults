static void on_handshake_complete(h2o_socket_t *sock, const char *err)
{
    h2o_socketpool_connect_request_t *req = sock->data;

    assert(req->sock == sock);

    if (err == h2o_socket_error_ssl_cert_name_mismatch && (SSL_CTX_get_verify_mode(req->pool->_ssl_ctx) & SSL_VERIFY_PEER) == 0) {
        /* ignore CN mismatch if we are not verifying peer */
    } else if (err != NULL) {
        h2o_socket_close(sock);
        req->sock = NULL;
    }

    call_connect_cb(req, err);
}


// Source: socketpool.c
// Lines 347-361
