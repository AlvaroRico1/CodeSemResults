static void aead_do_encrypt_init(ptls_aead_context_t *_ctx, uint64_t seq, const void *aad, size_t aadlen)
{
    struct aead_crypto_context_t *ctx = (struct aead_crypto_context_t *)_ctx;
    uint8_t iv[PTLS_MAX_IV_SIZE];
    int ret;

    ptls_aead__build_iv(ctx->super.algo, iv, ctx->static_iv, seq);
    ret = EVP_EncryptInit_ex(ctx->evp_ctx, NULL, NULL, NULL, iv);
    assert(ret);

    if (aadlen != 0) {
        int blocklen;
        ret = EVP_EncryptUpdate(ctx->evp_ctx, NULL, &blocklen, aad, (int)aadlen);
        assert(ret);
    }
}


// Source: openssl.c
// Lines 892-907
