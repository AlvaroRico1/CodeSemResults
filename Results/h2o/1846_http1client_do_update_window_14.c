static void do_update_window(h2o_httpclient_t *_client)
{
    struct st_h2o_http1client_t *client = (void *)_client;
    if ((*client->super.buf)->size >= client->super.ctx->max_buffer_size) {
        if (h2o_socket_is_reading(client->sock)) {
            client->reader = client->sock->_cb.read;
            h2o_socket_read_stop(client->sock);
        }
    } else {
        if (!h2o_socket_is_reading(client->sock)) {
            h2o_socket_read_start(client->sock, client->reader);
        }
    }
}


// Source: http1client.c
// Lines 773-786
