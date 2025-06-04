static int handle_certificate_verify(ptls_t *tls, ptls_iovec_t message, const char *context_string)
{
    const uint8_t *src = message.base + PTLS_HANDSHAKE_HEADER_SIZE, *const end = message.base + message.len;
    uint16_t algo;
    ptls_iovec_t signature;
    uint8_t signdata[PTLS_MAX_CERTIFICATE_VERIFY_SIGNDATA_SIZE];
    size_t signdata_size;
    int ret;

    /* decode */
    if ((ret = ptls_decode16(&algo, &src, end)) != 0)
        goto Exit;
    ptls_decode_block(src, end, 2, {
        signature = ptls_iovec_init(src, end - src);
        src = end;
    });

    signdata_size = build_certificate_verify_signdata(signdata, tls->key_schedule, context_string);
    if (tls->certificate_verify.cb != NULL) {
        ret = tls->certificate_verify.cb(tls->certificate_verify.verify_ctx, algo, ptls_iovec_init(signdata, signdata_size),
                                         signature);
    } else {
        ret = 0;
    }
    ptls_clear_memory(signdata, signdata_size);
    tls->certificate_verify.cb = NULL;
    if (ret != 0) {
        goto Exit;
    }

    ptls__key_schedule_update_hash(tls->key_schedule, message.base, message.len);

Exit:
    return ret;
}


// Source: picotls.c
// Lines 2867-2901
