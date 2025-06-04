static ptls_cipher_suite_t *get_aes128gcmsha256(quicly_context_t *ctx)
{
    ptls_cipher_suite_t **cs;

    for (cs = ctx->tls->cipher_suites;; ++cs) {
        assert(cs != NULL);
        if ((*cs)->id == PTLS_CIPHER_SUITE_AES_128_GCM_SHA256)
            break;
    }
    return *cs;
}


// Source: quicly.c
// Lines 533-543
