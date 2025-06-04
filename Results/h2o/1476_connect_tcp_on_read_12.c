static void tcp_on_read(h2o_socket_t *_sock, const char *err)
{
    struct st_connect_generator_t *self = _sock->data;

    h2o_socket_read_stop(self->sock);
    reset_io_timeout(self); /* for simplicity, we call out I/O timeout even when downstream fails to deliver data to the client
                             * within given interval */

    if (err == NULL) {
        h2o_iovec_t vec = h2o_iovec_init(self->sock->input->bytes, self->sock->input->size);
        h2o_send(self->src_req, &vec, 1, H2O_SEND_STATE_IN_PROGRESS);
    } else {
        /* unidirectional close is signalled using H2O_SEND_STATE_FINAL, but the write side remains open */
        self->read_closed = 1;
        h2o_send(self->src_req, NULL, 0, H2O_SEND_STATE_FINAL);
    }
}


// Source: connect.c
// Lines 524-540
