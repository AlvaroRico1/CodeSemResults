static int decode_new_session_ticket(ptls_t *tls, uint32_t *lifetime, uint32_t *age_add, ptls_iovec_t *nonce, ptls_iovec_t *ticket,
                                     uint32_t *max_early_data_size, const uint8_t *src, const uint8_t *const end)
{
    uint16_t exttype;
    int ret;

    if ((ret = ptls_decode32(lifetime, &src, end)) != 0)
        goto Exit;
    if ((ret = ptls_decode32(age_add, &src, end)) != 0)
        goto Exit;
    ptls_decode_open_block(src, end, 1, {
        *nonce = ptls_iovec_init(src, end - src);
        src = end;
    });
    ptls_decode_open_block(src, end, 2, {
        if (src == end) {
            ret = PTLS_ALERT_DECODE_ERROR;
            goto Exit;
        }
        *ticket = ptls_iovec_init(src, end - src);
        src = end;
    });

    *max_early_data_size = 0;
    decode_extensions(src, end, PTLS_HANDSHAKE_TYPE_NEW_SESSION_TICKET, &exttype, {
        if (tls->ctx->on_extension != NULL &&
            (ret = tls->ctx->on_extension->cb(tls->ctx->on_extension, tls, PTLS_HANDSHAKE_TYPE_NEW_SESSION_TICKET, exttype,
                                              ptls_iovec_init(src, end - src)) != 0))
            goto Exit;
        switch (exttype) {
        case PTLS_EXTENSION_TYPE_EARLY_DATA:
            if ((ret = ptls_decode32(max_early_data_size, &src, end)) != 0)
                goto Exit;
            break;
        default:
            src = end;
            break;
        }
    });

    ret = 0;
Exit:
    return ret;
}


// Source: picotls.c
// Lines 1082-1125
