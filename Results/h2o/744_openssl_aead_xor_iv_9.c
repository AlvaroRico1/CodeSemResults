static void aead_xor_iv(ptls_aead_context_t *_ctx, const void *_bytes, size_t len)
{
    struct aead_crypto_context_t *ctx = (struct aead_crypto_context_t *)_ctx;
    const uint8_t *bytes = _bytes;

    for (size_t i = 0; i < len; ++i)
        ctx->static_iv[i] ^= bytes[i];
}


// Source: openssl.c
// Lines 883-890
