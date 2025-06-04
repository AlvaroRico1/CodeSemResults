static int emit_compressed_certificate(ptls_emit_certificate_t *_self, ptls_t *tls, ptls_message_emitter_t *emitter,
                                       ptls_key_schedule_t *key_sched, ptls_iovec_t context, int push_status_request,
                                       const uint16_t *compress_algos, size_t num_compress_algos)
{
    ptls_emit_compressed_certificate_t *self = (void *)_self;
    struct st_ptls_compressed_certificate_entry_t *entry;
    int ret;

    assert(context.len == 0 || !"precompressed mode can only be used for server certificates");

    for (size_t i = 0; i != num_compress_algos; ++i) {
        if (compress_algos[i] == PTLS_CERTIFICATE_COMPRESSION_ALGORITHM_BROTLI)
            goto FoundBrotli;
    }
    /* brotli not found, delegate to the core */
    ret = PTLS_ERROR_DELEGATE;
    goto Exit;

FoundBrotli:
    entry = &self->without_ocsp_status;
    if (push_status_request && self->with_ocsp_status.uncompressed_length != 0)
        entry = &self->with_ocsp_status;

    ptls_push_message(emitter, key_sched, PTLS_HANDSHAKE_TYPE_COMPRESSED_CERTIFICATE, {
        ptls_buffer_push16(emitter->buf, PTLS_CERTIFICATE_COMPRESSION_ALGORITHM_BROTLI);
        ptls_buffer_push24(emitter->buf, entry->uncompressed_length);
        ptls_buffer_push_block(emitter->buf, 3, { ptls_buffer_pushv(emitter->buf, entry->bytes.base, entry->bytes.len); });
    });

    ret = 0;

Exit:
    return ret;
}


// Source: certificate_compression.c
// Lines 50-83
