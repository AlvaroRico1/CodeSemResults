int quicly_retry_calc_cidpair_hash(ptls_hash_algorithm_t *sha256, ptls_iovec_t client_cid, ptls_iovec_t server_cid, uint64_t *value)
{
    uint8_t digest[PTLS_SHA256_DIGEST_SIZE], buf[(QUICLY_MAX_CID_LEN_V1 + 1) * 2], *p = buf;
    int ret;

    *p++ = (uint8_t)client_cid.len;
    memcpy(p, client_cid.base, client_cid.len);
    p += client_cid.len;
    *p++ = (uint8_t)server_cid.len;
    memcpy(p, server_cid.base, server_cid.len);
    p += server_cid.len;

    if ((ret = ptls_calc_hash(sha256, digest, buf, p - buf)) != 0)
        return ret;
    p = digest;
    *value = quicly_decode64((void *)&p);

    return 0;
}


// Source: quicly.c
// Lines 3982-4000
