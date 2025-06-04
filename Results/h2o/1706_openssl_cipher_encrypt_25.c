static void cipher_encrypt(ptls_cipher_context_t *_ctx, void *output, const void *input, size_t _len)
{
    struct cipher_context_t *ctx = (struct cipher_context_t *)_ctx;
    int len = (int)_len, ret = EVP_EncryptUpdate(ctx->evp, output, &len, input, len);
    assert(ret);
    assert(len == (int)_len);
}


// Source: openssl.c
// Lines 815-821
