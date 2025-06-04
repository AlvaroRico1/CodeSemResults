    enum { HANDSHAKE_MODE_FULL, HANDSHAKE_MODE_PSK, HANDSHAKE_MODE_PSK_DHE } mode;
    size_t psk_index = SIZE_MAX;
    ptls_iovec_t pubkey = {0}, ecdh_secret = {0};
    int accept_early_data = 0, is_second_flight = tls->state == PTLS_STATE_SERVER_EXPECT_SECOND_CLIENT_HELLO, ret;

    if ((ch = malloc(sizeof(*ch))) == NULL) {
        ret = PTLS_ERROR_NO_MEMORY;
        goto Exit;
    }

    *ch = (struct st_ptls_client_hello_t){0,      NULL,   {NULL},     {NULL}, 0,     {NULL},   {NULL}, {NULL}, {{0}},
                                          {NULL}, {NULL}, {{{NULL}}}, {{0}},  {{0}}, {{NULL}}, {NULL}, {{0}},  {{UINT16_MAX}}};

    /* decode ClientHello */
    if ((ret = decode_client_hello(tls, ch, message.base + PTLS_HANDSHAKE_HEADER_SIZE, message.base + message.len, properties)) !=
        0)
        goto Exit;

    /* bail out if CH cannot be handled as TLS 1.3, providing the application the raw CH and SNI, to help them fallback */
    if (!is_supported_version(ch->selected_version)) {
        if (!is_second_flight && tls->ctx->on_client_hello != NULL) {
            ptls_on_client_hello_parameters_t params = {
                .server_name = ch->server_name,
                .raw_message = message,
                .negotiated_protocols = {ch->alpn.list, ch->alpn.count},
                .incompatible_version = 1,
            };
            if ((ret = tls->ctx->on_client_hello->cb(tls->ctx->on_client_hello, tls, &params)) != 0)
                goto Exit;
        }
        ret = PTLS_ALERT_PROTOCOL_VERSION;
        goto Exit;
    }

    /* Check TLS 1.3-specific constraints. Hereafter, we might exit without calling on_client_hello. That's fine because this CH is
     * ought to be rejected. */
    if (ch->legacy_version <= 0x0300) {
        /* RFC 8446 Appendix D.5: any endpoint receiving a Hello message with legacy_version set to 0x0300 MUST abort the handshake
         * with a "protocol_version" alert. */
        ret = PTLS_ALERT_PROTOCOL_VERSION;
        goto Exit;
    }
    if (!(ch->compression_methods.count == 1 && ch->compression_methods.ids[0] == 0)) {
        ret = PTLS_ALERT_ILLEGAL_PARAMETER;
        goto Exit;
    }
    /* esni */
    if (ch->esni.cipher != NULL) {
        if (ch->key_shares.base == NULL) {
            ret = PTLS_ALERT_ILLEGAL_PARAMETER;
            goto Exit;
        }
    }
    /* pre-shared key */
    if (ch->psk.hash_end != NULL) {
        /* PSK must be the last extension */
        if (!ch->psk.is_last_extension) {
            ret = PTLS_ALERT_ILLEGAL_PARAMETER;
            goto Exit;
        }
    } else {
        if (ch->psk.early_data_indication) {
            ret = PTLS_ALERT_ILLEGAL_PARAMETER;
            goto Exit;
        }
    }

    if (tls->ctx->require_dhe_on_psk)
        ch->psk.ke_modes &= ~(1u << PTLS_PSK_KE_MODE_PSK);

    /* handle client_random, legacy_session_id, SNI, ESNI */
    if (!is_second_flight) {
        memcpy(tls->client_random, ch->random_bytes, sizeof(tls->client_random));
        log_client_random(tls);
        if (ch->legacy_session_id.len != 0)
            tls->send_change_cipher_spec = 1;
        ptls_iovec_t server_name = {NULL};
        int is_esni = 0;
        if (ch->esni.cipher != NULL && tls->ctx->esni != NULL) {
            if ((ret = client_hello_decrypt_esni(tls->ctx, &server_name, &tls->esni, ch)) != 0)
                goto Exit;
            if (tls->ctx->update_esni_key != NULL) {
                if ((ret = tls->ctx->update_esni_key->cb(tls->ctx->update_esni_key, tls, tls->esni->secret, ch->esni.cipher->hash,
                                                         tls->esni->esni_contents_hash)) != 0)
                    goto Exit;
            }
            is_esni = 1;
        } else if (ch->server_name.base != NULL) {
            server_name = ch->server_name;
        }
        if (tls->ctx->on_client_hello != NULL) {
            ptls_on_client_hello_parameters_t params = {server_name,
                                                        message,
                                                        {ch->alpn.list, ch->alpn.count},
                                                        {ch->signature_algorithms.list, ch->signature_algorithms.count},
                                                        {ch->cert_compression_algos.list, ch->cert_compression_algos.count},
                                                        {ch->client_ciphers.list, ch->client_ciphers.count},
                                                        {ch->server_certificate_types.list, ch->server_certificate_types.count},
                                                        is_esni};
            ret = tls->ctx->on_client_hello->cb(tls->ctx->on_client_hello, tls, &params);
        } else {
            ret = 0;
        }

        if (is_esni)
            free(server_name.base);
        if (ret != 0)
            goto Exit;

        if (!certificate_type_exists(ch->server_certificate_types.list, ch->server_certificate_types.count,
                                     tls->ctx->use_raw_public_keys ? PTLS_CERTIFICATE_TYPE_RAW_PUBLIC_KEY
                                                                   : PTLS_CERTIFICATE_TYPE_X509)) {
            ret = PTLS_ALERT_UNSUPPORTED_CERTIFICATE;
            goto Exit;
        }
    } else {
        if (ch->psk.early_data_indication) {
            ret = PTLS_ALERT_DECODE_ERROR;
            goto Exit;
        }
        /* the following check is necessary so that we would be able to track the connection in SSLKEYLOGFILE, even though it
         * might not be for the safety of the protocol */
        if (!ptls_mem_equal(tls->client_random, ch->random_bytes, sizeof(tls->client_random))) {
            ret = PTLS_ALERT_HANDSHAKE_FAILURE;
            goto Exit;
        }
        /* We compare SNI only when the value is saved by the on_client_hello callback. This should be OK because we are
         * ignoring the value unless the callback saves the server-name. */
        if (tls->server_name != NULL) {
            size_t l = strlen(tls->server_name);
            if (!(ch->server_name.len == l && memcmp(ch->server_name.base, tls->server_name, l) == 0)) {
                ret = PTLS_ALERT_HANDSHAKE_FAILURE;
                goto Exit;
            }
        }
    }

    { /* select (or check) cipher-suite, create key_schedule */
        ptls_cipher_suite_t *cs;
        if ((ret = select_cipher(&cs, tls->ctx->cipher_suites, ch->cipher_suites.base,
                                 ch->cipher_suites.base + ch->cipher_suites.len, tls->ctx->server_cipher_preference)) != 0)
            goto Exit;
        if (!is_second_flight) {
            tls->cipher_suite = cs;
            tls->key_schedule = key_schedule_new(cs, NULL, tls->ctx->hkdf_label_prefix__obsolete);
        } else {
            if (tls->cipher_suite != cs) {
                ret = PTLS_ALERT_HANDSHAKE_FAILURE;
                goto Exit;
            }
        }
    }

    /* select key_share */
    if (key_share.algorithm == NULL && ch->key_shares.base != NULL) {
        const uint8_t *src = ch->key_shares.base, *const end = src + ch->key_shares.len;
        ptls_decode_block(src, end, 2, {
            if ((ret = select_key_share(&key_share.algorithm, &key_share.peer_key, tls->ctx->key_exchanges, &src, end, 0)) != 0)
                goto Exit;
        });
    }


// Source: picotls.c
// Lines 3692-3852
