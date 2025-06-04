static int decrypt_packet(ptls_cipher_context_t *header_protection,
                          int (*aead_cb)(void *, uint64_t, quicly_decoded_packet_t *, size_t, size_t *), void *aead_ctx,
                          uint64_t *next_expected_pn, quicly_decoded_packet_t *packet, uint64_t *pn, ptls_iovec_t *payload)
{
    int ret;

    /* decrypt ourselves, or use the pre-decrypted input */
    if (packet->decrypted.pn == UINT64_MAX) {
        if ((ret = do_decrypt_packet(header_protection, aead_cb, aead_ctx, next_expected_pn, packet, pn, payload)) != 0)
            return ret;
    } else {
        *payload = ptls_iovec_init(packet->octets.base + packet->encrypted_off, packet->octets.len - packet->encrypted_off);
        *pn = packet->decrypted.pn;
        if (aead_cb == aead_decrypt_1rtt) {
            quicly_conn_t *conn = aead_ctx;
            if (conn->application->cipher.ingress.key_phase.decrypted < packet->decrypted.key_phase) {
                if ((ret = received_key_update(conn, packet->decrypted.key_phase)) != 0)
                    return ret;
            }
        }


// Source: quicly.c
// Lines 2502-2521
