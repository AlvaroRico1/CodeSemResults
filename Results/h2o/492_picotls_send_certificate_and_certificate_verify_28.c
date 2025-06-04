static int send_certificate_and_certificate_verify(ptls_t *tls, ptls_message_emitter_t *emitter,
                                                   struct st_ptls_signature_algorithms_t *signature_algorithms,
                                                   ptls_iovec_t context, const char *context_string, int push_status_request,
                                                   const uint16_t *compress_algos, size_t num_compress_algos)
{
    int ret;

    if (signature_algorithms->count == 0) {
        ret = PTLS_ALERT_MISSING_EXTENSION;
        goto Exit;
    }

    { /* send Certificate (or the equivalent) */
        static ptls_emit_certificate_t default_emit_certificate = {default_emit_certificate_cb};
        ptls_emit_certificate_t *emit_certificate =
            tls->ctx->emit_certificate != NULL ? tls->ctx->emit_certificate : &default_emit_certificate;
    Redo:
        if ((ret = emit_certificate->cb(emit_certificate, tls, emitter, tls->key_schedule, context, push_status_request,
                                        compress_algos, num_compress_algos)) != 0) {
            if (ret == PTLS_ERROR_DELEGATE) {
                assert(emit_certificate != &default_emit_certificate);
                emit_certificate = &default_emit_certificate;
                goto Redo;
            }
            goto Exit;
        }
    }


// Source: picotls.c
// Lines 2665-2691
