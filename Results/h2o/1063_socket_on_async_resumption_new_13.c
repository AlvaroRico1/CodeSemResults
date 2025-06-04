static int on_async_resumption_new(SSL *ssl, SSL_SESSION *session)
{
    h2o_socket_t *sock = BIO_get_data(SSL_get_rbio(ssl));

    h2o_iovec_t data;
    const unsigned char *id;
    unsigned id_len;
    unsigned char *p;

    /* build data */
    data.len = i2d_SSL_SESSION(session, NULL);
    data.base = alloca(data.len);
    p = (void *)data.base;
    i2d_SSL_SESSION(session, &p);

    id = SSL_SESSION_get_id(session, &id_len);
    resumption_new(sock, h2o_iovec_init(id, id_len), data);
    return 0;
}


// Source: socket.c
// Lines 1230-1248
