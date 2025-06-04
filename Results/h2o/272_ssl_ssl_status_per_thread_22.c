static void ssl_status_per_thread(void *_ssc, h2o_context_t *ctx)
{
    struct st_ssl_status_ctx_t *ssc = _ssc;

    pthread_mutex_lock(&ssc->mutex);

    ssc->alpn_h1 += ctx->ssl.alpn_h1;
    ssc->alpn_h2 += ctx->ssl.alpn_h2;
    ssc->handshake_full += ctx->ssl.handshake_full;
    ssc->handshake_resume += ctx->ssl.handshake_resume;
    ssc->handshake_accum_time_full += ctx->ssl.handshake_accum_time_full;
    ssc->handshake_accum_time_resume += ctx->ssl.handshake_accum_time_resume;

    pthread_mutex_unlock(&ssc->mutex);
}


// Source: ssl.c
// Lines 36-50
