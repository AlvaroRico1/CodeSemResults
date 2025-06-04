static int x9_62_init_key(ptls_key_exchange_algorithm_t *algo, ptls_key_exchange_context_t **_ctx, EC_KEY *eckey)
{
    struct st_x9_62_keyex_context_t *ctx = NULL;
    int ret;

    if ((ret = x9_62_create_context(algo, &ctx)) != 0)
        goto Exit;
    ctx->privkey = eckey;
    if ((ret = x9_62_setup_pubkey(ctx)) != 0)
        goto Exit;
    ret = 0;

Exit:
    if (ret == 0) {
        *_ctx = &ctx->super;
    } else {
        if (ctx != NULL)
            x9_62_free_context(ctx);
        *_ctx = NULL;
    }
    return ret;
}


// Source: openssl.c
// Lines 362-383
