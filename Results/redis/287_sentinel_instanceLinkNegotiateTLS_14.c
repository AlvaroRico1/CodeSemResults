static int instanceLinkNegotiateTLS(redisAsyncContext *context) {
#ifndef USE_OPENSSL
    (void) context;
#else
    if (!redis_tls_ctx) return C_ERR;
    SSL *ssl = SSL_new(redis_tls_client_ctx ? redis_tls_client_ctx : redis_tls_ctx);
    if (!ssl) return C_ERR;

    if (redisInitiateSSL(&context->c, ssl) == REDIS_ERR) return C_ERR;
#endif
    return C_OK;
}


// Source: sentinel.c
// Lines 2397-2408
