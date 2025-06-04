static void on_connect(h2o_socket_t *sock, const char *err)
{
    h2o_socketpool_connect_request_t *req = sock->data;

    assert(req->sock == sock);

    if (err != NULL) {
        __sync_sub_and_fetch(&req->pool->targets.entries[req->selected_target]->_shared.leased_count, 1);
        h2o_socket_close(sock);
        if (req->remaining_try_count > 0) {
            try_connect(req);
            return;
        }
        __sync_sub_and_fetch(&req->pool->_shared.count, 1);
        req->sock = NULL;
    } else {
        h2o_url_t *target_url = &req->pool->targets.entries[req->selected_target]->url;
        if (target_url->scheme->is_ssl) {
            assert(req->pool->_ssl_ctx != NULL && "h2o_socketpool_set_ssl_ctx must be called for a pool that contains SSL target");
            h2o_socket_ssl_handshake(sock, req->pool->_ssl_ctx, target_url->host.base, req->alpn_protos, on_handshake_complete);
            return;
        }
    }

    call_connect_cb(req, err);
}


// Source: socketpool.c
// Lines 363-388
