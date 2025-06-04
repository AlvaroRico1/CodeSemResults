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


// Source: quicly.c
// Lines 2225-2247
