static int received_key_update(quicly_conn_t *conn, uint64_t newly_decrypted_key_phase)
{
    struct st_quicly_application_space_t *space = conn->application;

    assert(space->cipher.ingress.key_phase.decrypted < newly_decrypted_key_phase);
    assert(newly_decrypted_key_phase <= space->cipher.ingress.key_phase.prepared);

    space->cipher.ingress.key_phase.decrypted = newly_decrypted_key_phase;

    QUICLY_PROBE(CRYPTO_RECEIVE_KEY_UPDATE, conn, conn->stash.now, space->cipher.ingress.key_phase.decrypted,
                 QUICLY_PROBE_HEXDUMP(space->cipher.ingress.secret, ptls_get_cipher(conn->crypto.tls)->hash->digest_size));

    if (space->cipher.egress.key_phase < space->cipher.ingress.key_phase.decrypted) {
        return update_1rtt_egress_key(conn);
    } else {
        return 0;
    }
}


// Source: quicly.c
// Lines 1545-1562
