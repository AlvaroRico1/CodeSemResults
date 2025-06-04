static int tcp_write(void *_self, int is_end_stream)
{
    struct st_connect_generator_t *self = _self;
    h2o_iovec_t chunk = self->src_req->entity;

    assert(!self->write_closed);
    assert(self->tcp.sendbuf->size == 0);

    /* the socket might have been closed due to a read error */
    if (self->socket_closed)
        return 1;

    /* buffer input */
    h2o_buffer_append(&self->tcp.sendbuf, chunk.base, chunk.len);
    if (is_end_stream)
        self->write_closed = 1;

    /* write if the socket has been opened */
    if (self->sock != NULL && !h2o_socket_is_writing(self->sock))
        tcp_do_write(self);

    return 0;
}


// Source: connect.c
// Lines 500-522
