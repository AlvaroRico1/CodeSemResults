static int aead_decrypt_fixed_key(void *ctx, uint64_t pn, quicly_decoded_packet_t *packet, size_t aead_off, size_t *ptlen)
{
    ptls_aead_context_t *aead = ctx;

    if ((*ptlen = aead_decrypt_core(aead, pn, packet, aead_off)) == SIZE_MAX)
        return QUICLY_ERROR_PACKET_IGNORED;
    return 0;
}


// Source: quicly.c
// Lines 2406-2413
