static int handle_key_update(ptls_t *tls, ptls_message_emitter_t *emitter, ptls_iovec_t message)
{
    const uint8_t *src = message.base + PTLS_HANDSHAKE_HEADER_SIZE, *const end = message.base + message.len;
    int ret;

    /* validate */
    if (end - src != 1 || *src > 1)
        return PTLS_ALERT_DECODE_ERROR;

    /* update receive key */
    if ((ret = update_traffic_key(tls, 0)) != 0)
        return ret;

    if (*src) {
        if (tls->ctx->update_traffic_key != NULL)
            return PTLS_ALERT_UNEXPECTED_MESSAGE;
        tls->needs_key_update = 1;
    }

    return 0;
}


// Source: picotls.c
// Lines 4225-4245
