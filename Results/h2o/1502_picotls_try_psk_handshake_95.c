static int try_psk_handshake(ptls_t *tls, size_t *psk_index, int *accept_early_data, struct st_ptls_client_hello_t *ch,
                             ptls_iovec_t ch_trunc)
{
    ptls_buffer_t decbuf;
    ptls_iovec_t ticket_psk, ticket_server_name, ticket_negotiated_protocol;
    uint64_t issue_at, now = tls->ctx->get_time->cb(tls->ctx->get_time);
    uint32_t age_add;
    uint16_t ticket_key_exchange_id, ticket_csid;
    uint8_t binder_key[PTLS_MAX_DIGEST_SIZE];
    int ret;

    ptls_buffer_init(&decbuf, "", 0);

    for (*psk_index = 0; *psk_index < ch->psk.identities.count; ++*psk_index) {
        struct st_ptls_client_hello_psk_t *identity = ch->psk.identities.list + *psk_index;
        /* decrypt and decode */
        int can_accept_early_data = 1;
        decbuf.off = 0;
        switch (tls->ctx->encrypt_ticket->cb(tls->ctx->encrypt_ticket, tls, 0, &decbuf, identity->identity)) {
        case 0: /* decrypted */
            break;
        case PTLS_ERROR_REJECT_EARLY_DATA: /* decrypted, but early data is rejected */
            can_accept_early_data = 0;
            break;
        default: /* decryption failure */
            continue;
        }
        if (decode_session_identifier(&issue_at, &ticket_psk, &age_add, &ticket_server_name, &ticket_key_exchange_id, &ticket_csid,
                                      &ticket_negotiated_protocol, decbuf.base, decbuf.base + decbuf.off) != 0)
            continue;
        /* check age */
        if (now < issue_at)
            continue;
        if (now - issue_at > (uint64_t)tls->ctx->ticket_lifetime * 1000)
            continue;
        *accept_early_data = 0;
        if (ch->psk.early_data_indication && can_accept_early_data) {
            /* accept early-data if abs(diff) between the reported age and the actual age is within += 10 seconds */
            int64_t delta = (now - issue_at) - (identity->obfuscated_ticket_age - age_add);
            if (delta < 0)
                delta = -delta;
            if (tls->ctx->max_early_data_size != 0 && delta <= PTLS_EARLY_DATA_MAX_DELAY)
                *accept_early_data = 1;
        }
        /* check server-name */
        if (ticket_server_name.len != 0) {
            if (tls->server_name == NULL)
                continue;
            if (!vec_is_string(ticket_server_name, tls->server_name))
                continue;
        } else {
            if (tls->server_name != NULL)
                continue;
        }
        { /* check key-exchange */
            ptls_key_exchange_algorithm_t **a;
            for (a = tls->ctx->key_exchanges; *a != NULL && (*a)->id != ticket_key_exchange_id; ++a)
                ;
            if (*a == NULL)
                continue;
            tls->key_share = *a;
        }


// Source: picotls.c
// Lines 3488-3549
