void h2o_socket_ssl_handshake(h2o_socket_t *sock, SSL_CTX *ssl_ctx, const char *server_name, h2o_iovec_t alpn_protos,
                              h2o_socket_cb handshake_cb)
{
    sock->ssl = h2o_mem_alloc(sizeof(*sock->ssl));
    *sock->ssl = (struct st_h2o_socket_ssl_t){};

    sock->ssl->ssl_ctx = ssl_ctx;

    /* setup the buffers; sock->input should be empty, sock->ssl->input.encrypted should contain the initial input, if any */
    h2o_buffer_init(&sock->ssl->input.encrypted, &h2o_socket_buffer_prototype);
    if (sock->input->size != 0) {
        h2o_buffer_t *tmp = sock->input;
        sock->input = sock->ssl->input.encrypted;
        sock->ssl->input.encrypted = tmp;
    }


// Source: socket.c
// Lines 1534-1548
