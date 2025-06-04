static void on_dispose_ssl_ctx_session_cache(void *parent, void *ptr, CRYPTO_EX_DATA *ad, int idx, long argl, void *argp)
{
    h2o_cache_t *ssl_session_cache = (h2o_cache_t *)ptr;
    if (ssl_session_cache != NULL)
        h2o_cache_destroy(ssl_session_cache);
}


// Source: socket.c
// Lines 1642-1647
