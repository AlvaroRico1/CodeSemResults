static void cipher_dispose(ptls_cipher_context_t *_ctx)
{
    struct cipher_context_t *ctx = (struct cipher_context_t *)_ctx;
    EVP_CIPHER_CTX_free(ctx->evp);
}


// Source: openssl.c
// Lines 774-778
