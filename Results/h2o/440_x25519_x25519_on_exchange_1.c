static int x25519_on_exchange(ptls_key_exchange_context_t **_ctx, int release, ptls_iovec_t *secret, ptls_iovec_t peerkey)
{
    struct st_x25519_key_exchange_t *ctx = (struct st_x25519_key_exchange_t *)*_ctx;
    int ret;

    if (secret == NULL) {
        ret = 0;
        goto Exit;
    }

    if (peerkey.len != X25519_KEY_SIZE) {
        ret = PTLS_ALERT_DECRYPT_ERROR;
        goto Exit;
    }
    ret = x25519_derive_secret(secret, ctx->priv, ctx->pub, NULL, peerkey.base);

Exit:
    if (release) {
        ptls_clear_memory(ctx->priv, sizeof(ctx->priv));
        free(ctx);
        *_ctx = NULL;
    }
    return ret;
}


// Source: x25519.c
// Lines 52-75
