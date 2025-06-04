static void tcp_on_write_complete(h2o_socket_t *_sock, const char *err)
{
    struct st_connect_generator_t *self = _sock->data;

    /* until h2o_socket_t implements shutdown(SHUT_WR), do a bidirectional close when we close the write-side */
    if (err != NULL || self->write_closed) {
        close_readwrite(self);
        return;
    }

    reset_io_timeout(self);

    h2o_buffer_consume(&self->tcp.sendbuf, self->tcp.sendbuf->size);
    self->src_req->proceed_req(self->src_req, NULL);
}


// Source: connect.c
// Lines 476-490
