int decode_session_identifier(uint64_t *issued_at, ptls_iovec_t *psk, uint32_t *ticket_age_add, ptls_iovec_t *server_name,
                              uint16_t *key_exchange_id, uint16_t *csid, ptls_iovec_t *negotiated_protocol, const uint8_t *src,
                              const uint8_t *const end)
{
    int ret = 0;

    ptls_decode_block(src, end, 2, {
        if (end - src < SESSION_IDENTIFIER_MAGIC_SIZE ||
            memcmp(src, SESSION_IDENTIFIER_MAGIC, SESSION_IDENTIFIER_MAGIC_SIZE) != 0) {
            ret = PTLS_ALERT_DECODE_ERROR;
            goto Exit;
        }
        src += SESSION_IDENTIFIER_MAGIC_SIZE;
        if ((ret = ptls_decode64(issued_at, &src, end)) != 0)
            goto Exit;
        ptls_decode_open_block(src, end, 2, {
            *psk = ptls_iovec_init(src, end - src);
            src = end;
        });
        if ((ret = ptls_decode16(key_exchange_id, &src, end)) != 0)
            goto Exit;
        if ((ret = ptls_decode16(csid, &src, end)) != 0)
            goto Exit;
        if ((ret = ptls_decode32(ticket_age_add, &src, end)) != 0)
            goto Exit;
        ptls_decode_open_block(src, end, 2, {
            *server_name = ptls_iovec_init(src, end - src);
            src = end;
        });
        ptls_decode_open_block(src, end, 1, {
            *negotiated_protocol = ptls_iovec_init(src, end - src);
            src = end;
        });
    });

Exit:
    return ret;
}


// Source: picotls.c
// Lines 1299-1336
