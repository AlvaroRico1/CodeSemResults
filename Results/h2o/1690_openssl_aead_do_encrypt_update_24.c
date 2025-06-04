static size_t aead_do_encrypt_update(ptls_aead_context_t *_ctx, void *output, const void *input, size_t inlen)
{
    struct aead_crypto_context_t *ctx = (struct aead_crypto_context_t *)_ctx;
    int blocklen, ret;

    ret = EVP_EncryptUpdate(ctx->evp_ctx, output, &blocklen, input, (int)inlen);
    assert(ret);

    return blocklen;
}


// Source: openssl.c
// Lines 909-918
