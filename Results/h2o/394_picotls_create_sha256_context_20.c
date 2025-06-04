static ptls_hash_context_t *create_sha256_context(ptls_context_t *ctx)
{
    ptls_cipher_suite_t **cs;

    for (cs = ctx->cipher_suites; *cs != NULL; ++cs) {
        switch ((*cs)->id) {
        case PTLS_CIPHER_SUITE_AES_128_GCM_SHA256:
        case PTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256:
            return (*cs)->hash->create();
        }
    }

    return NULL;
}


// Source: picotls.c
// Lines 1558-1571
