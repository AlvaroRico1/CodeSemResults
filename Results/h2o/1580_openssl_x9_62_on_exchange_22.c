static int x9_62_on_exchange(ptls_key_exchange_context_t **_ctx, int release, ptls_iovec_t *secret, ptls_iovec_t peerkey)
{
    struct st_x9_62_keyex_context_t *ctx = (struct st_x9_62_keyex_context_t *)*_ctx;
    const EC_GROUP *group = EC_KEY_get0_group(ctx->privkey);
    EC_POINT *peer_point = NULL;
    int ret;

    if (secret == NULL) {
        ret = 0;
        goto Exit;
    }

    if ((peer_point = x9_62_decode_point(group, peerkey, ctx->bn_ctx)) == NULL) {
        ret = PTLS_ALERT_DECODE_ERROR;
        goto Exit;
    }
    if ((ret = ecdh_calc_secret(secret, group, ctx->privkey, peer_point)) != 0)
        goto Exit;

Exit:
    if (peer_point != NULL)
        EC_POINT_free(peer_point);
    if (release) {
        x9_62_free_context(ctx);
        *_ctx = NULL;
    }
    return ret;
}


// Source: openssl.c
// Lines 265-292
