static int udp_write_stream(void *_self, int is_end_stream)
{
    struct st_connect_generator_t *self = _self;
    h2o_iovec_t chunk = self->src_req->entity;

    assert(!self->write_closed);

    /* the socket might have been closed tue to a read error */
    if (self->socket_closed)
        return 1;

    if (is_end_stream)
        self->write_closed = 1;

    /* if the socket is not yet open, buffer input and return */
    if (self->sock == NULL) {
        h2o_buffer_append(&self->udp.egress.buf, chunk.base, chunk.len);
        return 0;
    }

    udp_do_write_stream(self, chunk);
    return 0;
}


// Source: connect.c
// Lines 687-709
