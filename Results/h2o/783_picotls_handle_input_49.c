static int handle_input(ptls_t *tls, ptls_message_emitter_t *emitter, ptls_buffer_t *decryptbuf, const void *input, size_t *inlen,
                        ptls_handshake_properties_t *properties)
{
    struct st_ptls_record_t rec;
    int ret;

    /* extract the record */
    if ((ret = parse_record(tls, &rec, input, inlen)) != 0)
        return ret;
    assert(rec.fragment != NULL);

    /* decrypt the record */
    if (rec.type == PTLS_CONTENT_TYPE_CHANGE_CIPHER_SPEC) {
        if (tls->state < PTLS_STATE_POST_HANDSHAKE_MIN) {
            if (!(rec.length == 1 && rec.fragment[0] == 0x01))
                return PTLS_ALERT_ILLEGAL_PARAMETER;
        } else {
            return PTLS_ALERT_HANDSHAKE_FAILURE;
        }
        ret = PTLS_ERROR_IN_PROGRESS;
        goto NextRecord;
    }
    if (tls->traffic_protection.dec.aead != NULL && rec.type != PTLS_CONTENT_TYPE_ALERT) {
        size_t decrypted_length;
        if (rec.type != PTLS_CONTENT_TYPE_APPDATA)
            return PTLS_ALERT_HANDSHAKE_FAILURE;
        if ((ret = ptls_buffer_reserve(decryptbuf, 5 + rec.length)) != 0)
            return ret;
        if ((ret = aead_decrypt(&tls->traffic_protection.dec, decryptbuf->base + decryptbuf->off, &decrypted_length, rec.fragment,
                                rec.length)) != 0) {
            if (tls->is_server && tls->server.early_data_skipped_bytes != UINT32_MAX)
                goto ServerSkipEarlyData;
            return ret;
        }
        rec.length = decrypted_length;
        rec.fragment = decryptbuf->base + decryptbuf->off;
        /* skip padding */
        for (; rec.length != 0; --rec.length)
            if (rec.fragment[rec.length - 1] != 0)
                break;
        if (rec.length == 0)
            return PTLS_ALERT_UNEXPECTED_MESSAGE;
        rec.type = rec.fragment[--rec.length];
    } else if (rec.type == PTLS_CONTENT_TYPE_APPDATA && tls->is_server && tls->server.early_data_skipped_bytes != UINT32_MAX) {
        goto ServerSkipEarlyData;
    }

    if (tls->recvbuf.mess.base != NULL || rec.type == PTLS_CONTENT_TYPE_HANDSHAKE) {
        /* handshake record */
        ret = handle_handshake_record(tls, tls->is_server ? handle_server_handshake_message : handle_client_handshake_message,
                                      emitter, &rec, properties);
    } else {
        /* handling of an alert or an application record */
        switch (rec.type) {
        case PTLS_CONTENT_TYPE_APPDATA:
            if (tls->state >= PTLS_STATE_POST_HANDSHAKE_MIN) {
                decryptbuf->off += rec.length;
                ret = 0;
            } else if (tls->state == PTLS_STATE_SERVER_EXPECT_END_OF_EARLY_DATA) {
                if (tls->traffic_protection.dec.aead != NULL)
                    decryptbuf->off += rec.length;
                ret = 0;
            } else {
                ret = PTLS_ALERT_UNEXPECTED_MESSAGE;
            }
            break;
        case PTLS_CONTENT_TYPE_ALERT:
            ret = handle_alert(tls, rec.fragment, rec.length);
            break;
        default:
            ret = PTLS_ALERT_UNEXPECTED_MESSAGE;
            break;
        }
    }

NextRecord:
    ptls_buffer_dispose(&tls->recvbuf.rec);
    return ret;

ServerSkipEarlyData:
    tls->server.early_data_skipped_bytes += (uint32_t)rec.length;
    if (tls->server.early_data_skipped_bytes > PTLS_MAX_EARLY_DATA_SKIP_SIZE)
        return PTLS_ALERT_HANDSHAKE_FAILURE;
    ret = PTLS_ERROR_IN_PROGRESS;
    goto NextRecord;
}


// Source: picotls.c
// Lines 4735-4820
