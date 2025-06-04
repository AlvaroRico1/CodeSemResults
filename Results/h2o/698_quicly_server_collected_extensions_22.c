static int server_collected_extensions(ptls_t *tls, ptls_handshake_properties_t *properties, ptls_raw_extension_t *slots)
{
    quicly_conn_t *conn = (void *)((char *)properties - offsetof(quicly_conn_t, crypto.handshake_properties));
    quicly_cid_t initial_scid;
    int ret;

    if (slots[0].type == UINT16_MAX) {
        ret = PTLS_ALERT_MISSING_EXTENSION;
        goto Exit;
    }
    assert(slots[0].type == get_transport_parameters_extension_id(conn->super.version));
    assert(slots[1].type == UINT16_MAX);

    { /* decode transport_parameters extension */
        const uint8_t *src = slots[0].data.base, *end = src + slots[0].data.len;
        if ((ret = quicly_decode_transport_parameter_list(&conn->super.remote.transport_params,
                                                          needs_cid_auth(conn) ? NULL : &tp_cid_ignore,
                                                          needs_cid_auth(conn) ? &initial_scid : &tp_cid_ignore,
                                                          needs_cid_auth(conn) ? NULL : &tp_cid_ignore, NULL, src, end)) != 0)
            goto Exit;
        if (needs_cid_auth(conn) &&
            !quicly_cid_is_equal(&conn->super.remote.cid_set.cids[0].cid, ptls_iovec_init(initial_scid.cid, initial_scid.len))) {
            ret = QUICLY_TRANSPORT_ERROR_PROTOCOL_VIOLATION;
            goto Exit;
        }
    }


// Source: quicly.c
// Lines 2334-2359
