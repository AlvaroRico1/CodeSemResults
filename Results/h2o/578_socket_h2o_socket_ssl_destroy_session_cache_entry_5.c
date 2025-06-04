void h2o_socket_ssl_destroy_session_cache_entry(h2o_iovec_t value)
{
    SSL_SESSION *session = (SSL_SESSION *)value.base;
    SSL_SESSION_free(session);
}


// Source: socket.c
// Lines 1666-1670
