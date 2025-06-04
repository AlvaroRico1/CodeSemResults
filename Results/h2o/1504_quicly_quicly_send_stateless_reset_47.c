size_t quicly_send_stateless_reset(quicly_context_t *ctx, const void *src_cid, void *payload)
{
    uint8_t *base = payload;

    /* build stateless reset packet */
    ctx->tls->random_bytes(base, QUICLY_STATELESS_RESET_PACKET_MIN_LEN - QUICLY_STATELESS_RESET_TOKEN_LEN);
    base[0] = (base[0] & ~QUICLY_LONG_HEADER_BIT) | QUICLY_QUIC_BIT;
    if (!ctx->cid_encryptor->generate_stateless_reset_token(
            ctx->cid_encryptor, base + QUICLY_STATELESS_RESET_PACKET_MIN_LEN - QUICLY_STATELESS_RESET_TOKEN_LEN, src_cid))
        return SIZE_MAX;

    return QUICLY_STATELESS_RESET_PACKET_MIN_LEN;
}


// Source: quicly.c
// Lines 4688-4700
