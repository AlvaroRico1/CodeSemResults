static void on_send_complete(h2o_socket_t *sock, const char *err)
{
    struct st_fcgi_generator_t *generator = sock->data;

    set_timeout(generator, generator->handler->config.io_timeout, on_rw_timeout);
    /* do nothing else!  all the rest is handled by the on_read */
}


// Source: fastcgi.c
// Lines 727-733
