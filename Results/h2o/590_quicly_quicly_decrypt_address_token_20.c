int quicly_decrypt_address_token(ptls_aead_context_t *aead, quicly_address_token_plaintext_t *plaintext, const void *_token,
                                 size_t len, size_t prefix_len, const char **err_desc)
{
    const uint8_t *const token = _token;
    uint8_t ptbuf[QUICLY_MIN_CLIENT_INITIAL_SIZE];
    size_t ptlen;

    *err_desc = NULL;

    /* check if we can get type and decrypt */
    if (len < prefix_len + 1 + aead->algo->iv_size + aead->algo->tag_size) {
        *err_desc = "token too small";
        return PTLS_ALERT_DECODE_ERROR;
    }
    if (prefix_len + 1 + aead->algo->iv_size + sizeof(ptbuf) + aead->algo->tag_size < len) {
        *err_desc = "token too large";
        return PTLS_ALERT_DECODE_ERROR;
    }

    /* check type */
    switch (token[prefix_len]) {
    case QUICLY_ADDRESS_TOKEN_TYPE_RETRY:
        plaintext->type = QUICLY_ADDRESS_TOKEN_TYPE_RETRY;
        break;
    case QUICLY_ADDRESS_TOKEN_TYPE_RESUMPTION:
        plaintext->type = QUICLY_ADDRESS_TOKEN_TYPE_RESUMPTION;
        break;
    default:
        *err_desc = "unknown token type";
        return PTLS_ALERT_DECODE_ERROR;
    }

    /* `goto Exit` can only happen below this line, and that is guaranteed by declaring `ret` here */
    int ret;

    /* decrypt */
    aead->algo->setup_crypto(aead, 0, NULL, token + prefix_len + 1);
    if ((ptlen = ptls_aead_decrypt(aead, ptbuf, token + prefix_len + 1 + aead->algo->iv_size,
                                   len - (prefix_len + 1 + aead->algo->iv_size), 0, token, prefix_len + 1 + aead->algo->iv_size)) ==
        SIZE_MAX) {
        ret = PTLS_ALERT_DECRYPT_ERROR;
        *err_desc = "token decryption failure";
        goto Exit;
    }

    /* parse */
    const uint8_t *src = ptbuf, *end = src + ptlen;
    if ((ret = ptls_decode64(&plaintext->issued_at, &src, end)) != 0)
        goto Exit;
    {
        in_port_t *portaddr;
        ptls_decode_open_block(src, end, 1, {
            switch (end - src) {
            case 4: /* ipv4 */
                plaintext->remote.sin.sin_family = AF_INET;
                memcpy(&plaintext->remote.sin.sin_addr.s_addr, src, 4);
                portaddr = &plaintext->remote.sin.sin_port;
                break;
            case 16: /* ipv6 */
                plaintext->remote.sin6.sin6_family = AF_INET6;
                memcpy(&plaintext->remote.sin6.sin6_addr, src, 16);
                portaddr = &plaintext->remote.sin6.sin6_port;
                break;
            default:
                ret = PTLS_ALERT_DECODE_ERROR;
                goto Exit;
            }
            src = end;
        });
        uint16_t port;
        if ((ret = ptls_decode16(&port, &src, end)) != 0)
            goto Exit;
        *portaddr = htons(port);
    }


// Source: quicly.c
// Lines 6305-6378
