static void call_connect_cb(h2o_socketpool_connect_request_t *req, const char *errstr)
{
    h2o_socketpool_connect_cb cb = req->cb;
    h2o_socket_t *sock = req->sock;
    void *data = req->data;
    h2o_socketpool_target_t *selected_target = req->pool->targets.entries[req->selected_target];

    if (req->lb.tried != NULL) {
        free(req->lb.tried);
    }

    free(req);

    if (sock != NULL)
        sock->data = NULL;
    cb(sock, errstr, data, &selected_target->url);
}


// Source: socketpool.c
// Lines 298-314
