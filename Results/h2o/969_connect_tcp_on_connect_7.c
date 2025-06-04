static void tcp_on_connect(h2o_socket_t *_sock, const char *err)
{
    struct st_connect_generator_t *self = _sock->data;

    assert(self->sock == _sock);

    if (err != NULL) {
        set_last_error(self, ERROR_CLASS_CONNECT, err);
        h2o_socket_close(self->sock);
        self->sock = NULL;
        try_connect(self);
        return;
    }

    stop_eyeballs(self);
    self->timeout.cb = on_io_timeout;
    reset_io_timeout(self);

    /* start the write if there's data to be sent */
    if (self->tcp.sendbuf->size != 0 || self->write_closed)
        tcp_do_write(self);

    /* build and submit 200 response */
    self->src_req->res.status = 200;
    h2o_start_response(self->src_req, &self->super);
    h2o_send(self->src_req, NULL, 0, H2O_SEND_STATE_IN_PROGRESS);
}


// Source: connect.c
// Lines 558-584
