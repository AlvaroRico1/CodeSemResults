static int client_handle_compressed_certificate(ptls_t *tls, ptls_iovec_t message)
{
    const uint8_t *src = message.base + PTLS_HANDSHAKE_HEADER_SIZE, *const end = message.base + message.len;
    uint16_t algo;
    uint32_t uncompressed_size;
    uint8_t *uncompressed = NULL;
    int ret;

    if (tls->ctx->decompress_certificate == NULL) {
        ret = PTLS_ALERT_UNEXPECTED_MESSAGE;
        goto Exit;
    }

    /* decode */
    if ((ret = ptls_decode16(&algo, &src, end)) != 0)
        goto Exit;
    if ((ret = ptls_decode24(&uncompressed_size, &src, end)) != 0)
        goto Exit;
    if (uncompressed_size > 65536) { /* TODO find a sensible number */
        ret = PTLS_ALERT_BAD_CERTIFICATE;
        goto Exit;
    }
    if ((uncompressed = malloc(uncompressed_size)) == NULL) {
        ret = PTLS_ERROR_NO_MEMORY;
        goto Exit;
    }
    ptls_decode_block(src, end, 3, {
        if ((ret = tls->ctx->decompress_certificate->cb(tls->ctx->decompress_certificate, tls, algo,
                                                        ptls_iovec_init(uncompressed, uncompressed_size),
                                                        ptls_iovec_init(src, end - src))) != 0)
            goto Exit;
        src = end;
    });

    /* handle */
    if ((ret = client_do_handle_certificate(tls, uncompressed, uncompressed + uncompressed_size)) != 0)
        goto Exit;

    ptls__key_schedule_update_hash(tls->key_schedule, message.base, message.len);
    tls->state = PTLS_STATE_CLIENT_EXPECT_CERTIFICATE_VERIFY;
    ret = PTLS_ERROR_IN_PROGRESS;

Exit:
    free(uncompressed);
    return ret;
}


// Source: picotls.c
// Lines 2805-2850
