int quicly_connect(quicly_conn_t **_conn, quicly_context_t *ctx, const char *server_name, struct sockaddr *dest_addr,
                   struct sockaddr *src_addr, const quicly_cid_plaintext_t *new_cid, ptls_iovec_t address_token,
                   ptls_handshake_properties_t *handshake_properties, const quicly_transport_parameters_t *resumed_transport_params)
{
    const struct st_ptls_salt_t *salt;
    quicly_conn_t *conn = NULL;
    const quicly_cid_t *server_cid;
    ptls_buffer_t buf;
    size_t epoch_offsets[5] = {0};
    size_t max_early_data_size = 0;
    int ret;

    if ((salt = get_salt(ctx->initial_version)) == NULL) {
        if ((ctx->initial_version & 0x0f0f0f0f) == 0x0a0a0a0a) {
            /* greasing version, use our own greasing salt */
            static const struct st_ptls_salt_t grease_salt = {.initial = {0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe,
                                                                          0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad,
                                                                          0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef}};
            salt = &grease_salt;
        } else {
            ret = QUICLY_ERROR_INVALID_INITIAL_VERSION;
            goto Exit;
        }
    }

    if ((conn = create_connection(
             ctx, ctx->initial_version, server_name, dest_addr, src_addr, NULL, new_cid, handshake_properties,
             quicly_cc_calc_initial_cwnd(ctx->initcwnd_packets, ctx->transport_params.max_udp_payload_size))) == NULL) {
        ret = PTLS_ERROR_NO_MEMORY;
        goto Exit;
    }
    conn->super.remote.address_validation.validated = 1;
    conn->super.remote.address_validation.send_probe = 1;
    if (address_token.len != 0) {
        if ((conn->token.base = malloc(address_token.len)) == NULL) {
            ret = PTLS_ERROR_NO_MEMORY;
            goto Exit;
        }
        memcpy(conn->token.base, address_token.base, address_token.len);
        conn->token.len = address_token.len;
    }
    server_cid = quicly_get_remote_cid(conn);
    conn->super.original_dcid = *server_cid;

    QUICLY_PROBE(CONNECT, conn, conn->stash.now, conn->super.version);

    if ((ret = setup_handshake_space_and_flow(conn, QUICLY_EPOCH_INITIAL)) != 0)
        goto Exit;
    if ((ret = setup_initial_encryption(get_aes128gcmsha256(ctx), &conn->initial->cipher.ingress, &conn->initial->cipher.egress,
                                        ptls_iovec_init(server_cid->cid, server_cid->len), 1,
                                        ptls_iovec_init(salt->initial, sizeof(salt->initial)), conn)) != 0)
        goto Exit;

    /* handshake (we always encode authentication CIDs, as we do not (yet) regenerate ClientHello when receiving Retry) */
    ptls_buffer_init(&conn->crypto.transport_params.buf, "", 0);
    if ((ret = quicly_encode_transport_parameter_list(
             &conn->crypto.transport_params.buf, &conn->super.ctx->transport_params, NULL, &conn->super.local.cid_set.cids[0].cid,
             NULL, NULL, conn->super.ctx->expand_client_hello ? conn->super.ctx->initial_egress_max_udp_payload_size : 0)) != 0)
        goto Exit;
    conn->crypto.transport_params.ext[0] =
        (ptls_raw_extension_t){QUICLY_TLS_EXTENSION_TYPE_TRANSPORT_PARAMETERS_FINAL,
                               {conn->crypto.transport_params.buf.base, conn->crypto.transport_params.buf.off}};
    conn->crypto.transport_params.ext[1] =
        (ptls_raw_extension_t){QUICLY_TLS_EXTENSION_TYPE_TRANSPORT_PARAMETERS_DRAFT,
                               {conn->crypto.transport_params.buf.base, conn->crypto.transport_params.buf.off}};
    conn->crypto.transport_params.ext[2] = (ptls_raw_extension_t){UINT16_MAX};
    conn->crypto.handshake_properties.additional_extensions = conn->crypto.transport_params.ext;
    conn->crypto.handshake_properties.collected_extensions = client_collected_extensions;

    ptls_buffer_init(&buf, "", 0);
    if (resumed_transport_params != NULL)
        conn->crypto.handshake_properties.client.max_early_data_size = &max_early_data_size;
    ret = ptls_handle_message(conn->crypto.tls, &buf, epoch_offsets, 0, NULL, 0, &conn->crypto.handshake_properties);
    conn->crypto.handshake_properties.client.max_early_data_size = NULL;
    if (ret != PTLS_ERROR_IN_PROGRESS) {
        assert(ret > 0); /* no QUIC errors */
        goto Exit;
    }
    write_crypto_data(conn, &buf, epoch_offsets);
    ptls_buffer_dispose(&buf);

    if (max_early_data_size != 0) {
        /* when attempting 0-RTT, apply the remembered transport parameters */
#define APPLY(n) conn->super.remote.transport_params.n = resumed_transport_params->n
        APPLY(active_connection_id_limit);
        APPLY(max_data);
        APPLY(max_stream_data.bidi_local);
        APPLY(max_stream_data.bidi_remote);
        APPLY(max_stream_data.uni);
        APPLY(max_streams_bidi);
        APPLY(max_streams_uni);
#undef APPLY
        if ((ret = apply_remote_transport_params(conn)) != 0)
            goto Exit;
    }

    *_conn = conn;
    ret = 0;

Exit:
    if (conn != NULL)
        unlock_now(conn);
    if (ret != 0) {
        if (conn != NULL)
            quicly_free(conn);
    }
    return ret;
}


// Source: quicly.c
// Lines 2225-2332
