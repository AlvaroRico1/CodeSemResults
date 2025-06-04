static int verify_cert(ptls_verify_certificate_t *_self, ptls_t *tls,
                       int (**verifier)(void *, uint16_t, ptls_iovec_t, ptls_iovec_t), void **verify_data, ptls_iovec_t *certs,
                       size_t num_certs)
{
    ptls_openssl_verify_certificate_t *self = (ptls_openssl_verify_certificate_t *)_self;
    X509 *cert = NULL;
    STACK_OF(X509) *chain = sk_X509_new_null();
    size_t i;
    int ret = 0;

    assert(num_certs != 0);

    /* convert certificates to OpenSSL representation */
    if ((cert = to_x509(certs[0])) == NULL) {
        ret = PTLS_ALERT_BAD_CERTIFICATE;
        goto Exit;
    }
    for (i = 1; i != num_certs; ++i) {
        X509 *interm = to_x509(certs[i]);
        if (interm == NULL) {
            ret = PTLS_ALERT_BAD_CERTIFICATE;
            goto Exit;
        }
        sk_X509_push(chain, interm);
    }

    /* verify the chain */
    if ((ret = verify_cert_chain(self->cert_store, cert, chain, ptls_is_server(tls), ptls_get_server_name(tls))) != 0)
        goto Exit;

    /* extract public key for verifying the TLS handshake signature */
    if ((*verify_data = X509_get_pubkey(cert)) == NULL) {
        ret = PTLS_ALERT_BAD_CERTIFICATE;
        goto Exit;
    }
    *verifier = verify_sign;

Exit:
    if (chain != NULL)
        sk_X509_pop_free(chain, X509_free);
    if (cert != NULL)
        X509_free(cert);
    return ret;
}


// Source: openssl.c
// Lines 1287-1330
