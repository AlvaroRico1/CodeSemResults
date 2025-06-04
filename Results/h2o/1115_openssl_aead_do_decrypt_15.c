static size_t aead_do_decrypt(ptls_aead_context_t *_ctx, void *_output, const void *input, size_t inlen, uint64_t seq,
                              const void *aad, size_t aadlen)
{
    struct aead_crypto_context_t *ctx = (struct aead_crypto_context_t *)_ctx;
    uint8_t *output = _output, iv[PTLS_MAX_IV_SIZE];
    size_t off = 0, tag_size = ctx->super.algo->tag_size;
    int blocklen, ret;

    if (inlen < tag_size)
        return SIZE_MAX;

    ptls_aead__build_iv(ctx->super.algo, iv, ctx->static_iv, seq);
    ret = EVP_DecryptInit_ex(ctx->evp_ctx, NULL, NULL, NULL, iv);
    assert(ret);
    if (aadlen != 0) {
        ret = EVP_DecryptUpdate(ctx->evp_ctx, NULL, &blocklen, aad, (int)aadlen);
        assert(ret);
    }
    ret = EVP_DecryptUpdate(ctx->evp_ctx, output + off, &blocklen, input, (int)(inlen - tag_size));
    assert(ret);
    off += blocklen;
    if (!EVP_CIPHER_CTX_ctrl(ctx->evp_ctx, EVP_CTRL_GCM_SET_TAG, (int)tag_size, (void *)((uint8_t *)input + inlen - tag_size)))
        return SIZE_MAX;
    if (!EVP_DecryptFinal_ex(ctx->evp_ctx, output + off, &blocklen))
        return SIZE_MAX;
    off += blocklen;

    return off;
}


// Source: openssl.c
// Lines 937-965
