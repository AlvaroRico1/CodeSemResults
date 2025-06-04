static void start_connect(h2o_socketpool_connect_request_t *req, struct sockaddr *addr, socklen_t addrlen)
{
    struct on_close_data_t *close_data;

    req->sock = h2o_socket_connect(req->loop, addr, addrlen, on_connect, NULL);
    if (req->sock == NULL) {
        __sync_sub_and_fetch(&req->pool->targets.entries[req->selected_target]->_shared.leased_count, 1);
        if (req->remaining_try_count > 0) {
            try_connect(req);
            return;
        }
        __sync_sub_and_fetch(&req->pool->_shared.count, 1);
        call_connect_cb(req, h2o_socket_error_conn_fail);
        return;
    }
    close_data = h2o_mem_alloc(sizeof(*close_data));
    close_data->pool = req->pool;
    close_data->target = req->selected_target;
    req->sock->data = req;
    req->sock->on_close.cb = on_close;
    req->sock->on_close.data = close_data;
}


// Source: socketpool.c
// Lines 399-420
