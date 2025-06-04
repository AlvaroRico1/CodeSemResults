static void accept_timeout(struct st_h2o_accept_data_t *data)
{
    /* TODO log */
    h2o_socket_t *sock = data->sock;
    accept_data_callbacks.destroy(data);
    h2o_socket_close(sock);
}


// Source: util.c
// Lines 304-310
