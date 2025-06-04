static int sign_certificate(ptls_sign_certificate_t *_self, ptls_t *tls, uint16_t *selected_algorithm, ptls_buffer_t *outbuf,
                            ptls_iovec_t input, const uint16_t *algorithms, size_t num_algorithms)
{
    ptls_openssl_sign_certificate_t *self = (ptls_openssl_sign_certificate_t *)_self;
    const struct st_ptls_openssl_signature_scheme_t *scheme;

    /* select the algorithm (driven by server-side preference of `self->schemes`) */
    for (scheme = self->schemes; scheme->scheme_id != UINT16_MAX; ++scheme) {
        size_t i;
        for (i = 0; i != num_algorithms; ++i)
            if (algorithms[i] == scheme->scheme_id)
                goto Found;
    }
    return PTLS_ALERT_HANDSHAKE_FAILURE;

Found:
    *selected_algorithm = scheme->scheme_id;
    return do_sign(self->key, scheme, outbuf, input);
}


// Source: openssl.c
// Lines 1042-1060
