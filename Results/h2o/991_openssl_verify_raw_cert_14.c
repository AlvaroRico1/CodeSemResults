static int verify_raw_cert(ptls_verify_certificate_t *_self, ptls_t *tls,
                           int (**verifier)(void *, uint16_t algo, ptls_iovec_t, ptls_iovec_t), void **verify_data,
                           ptls_iovec_t *certs, size_t num_certs)
{
    ptls_openssl_raw_pubkey_verify_certificate_t *self = (ptls_openssl_raw_pubkey_verify_certificate_t *)_self;
    int ret = PTLS_ALERT_BAD_CERTIFICATE;
    ptls_iovec_t expected_pubkey = {0};

    assert(num_certs != 0);

    if (num_certs != 1)
        goto Exit;

    int r = i2d_PUBKEY(self->expected_pubkey, &expected_pubkey.base);
    if (r <= 0) {
        ret = PTLS_ALERT_BAD_CERTIFICATE;
        goto Exit;
    }

    expected_pubkey.len = r;
    if (certs[0].len != expected_pubkey.len)
        goto Exit;

    if (!ptls_mem_equal(expected_pubkey.base, certs[0].base, certs[0].len))
        goto Exit;

    EVP_PKEY_up_ref(self->expected_pubkey);
    *verify_data = self->expected_pubkey;
    *verifier = verify_sign;
    ret = 0;
Exit:
    free(expected_pubkey.base);
    return ret;
}


// Source: openssl.c
// Lines 1374-1407
