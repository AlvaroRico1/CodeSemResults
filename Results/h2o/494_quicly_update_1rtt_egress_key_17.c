static int update_1rtt_egress_key(quicly_conn_t *conn)
{
    struct st_quicly_application_space_t *space = conn->application;
    ptls_cipher_suite_t *cipher = ptls_get_cipher(conn->crypto.tls);
    int ret;

    /* generate next AEAD key, and increment key phase if it succeeds */
    if ((ret = update_1rtt_key(conn, cipher, 1, &space->cipher.egress.key.aead, space->cipher.egress.secret)) != 0)
        return ret;
    ++space->cipher.egress.key_phase;

    /* signal that we are waiting for an ACK */
    space->cipher.egress.key_update_pn.last = conn->egress.packet_number;
    space->cipher.egress.key_update_pn.next = UINT64_MAX;

    QUICLY_PROBE(CRYPTO_SEND_KEY_UPDATE, conn, conn->stash.now, space->cipher.egress.key_phase,
                 QUICLY_PROBE_HEXDUMP(space->cipher.egress.secret, cipher->hash->digest_size));

    return 0;
}


// Source: quicly.c
// Lines 1524-1543
