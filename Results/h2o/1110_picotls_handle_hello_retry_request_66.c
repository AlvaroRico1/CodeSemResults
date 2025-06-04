static int handle_hello_retry_request(ptls_t *tls, ptls_message_emitter_t *emitter, struct st_ptls_server_hello_t *sh,
                                      ptls_iovec_t message, ptls_handshake_properties_t *properties)
{
    int ret;

    if (tls->client.key_share_ctx != NULL) {
        tls->client.key_share_ctx->on_exchange(&tls->client.key_share_ctx, 1, NULL, ptls_iovec_init(NULL, 0));
        tls->client.key_share_ctx = NULL;
    }
    if (tls->client.using_early_data) {
        /* release traffic encryption key so that 2nd CH goes out in cleartext, but keep the epoch at 1 since we've already
         * called derive-secret */
        if (tls->ctx->update_traffic_key == NULL) {
            assert(tls->traffic_protection.enc.aead != NULL);
            ptls_aead_free(tls->traffic_protection.enc.aead);
            tls->traffic_protection.enc.aead = NULL;
        }
        tls->client.using_early_data = 0;
    }

    if (sh->retry_request.selected_group != UINT16_MAX) {
        /* we offer the first key_exchanges[0] as KEY_SHARE unless client.negotiate_before_key_exchange is set */
        ptls_key_exchange_algorithm_t **cand;
        for (cand = tls->ctx->key_exchanges; *cand != NULL; ++cand)
            if ((*cand)->id == sh->retry_request.selected_group)
                break;
        if (*cand == NULL) {
            ret = PTLS_ALERT_ILLEGAL_PARAMETER;
            goto Exit;
        }
        tls->key_share = *cand;
    } else if (tls->key_share != NULL) {
        /* retain the key-share using in first CH, if server does not specify one */
    } else {
        ret = PTLS_ALERT_ILLEGAL_PARAMETER;
        goto Exit;
    }


// Source: picotls.c
// Lines 2309-2345
