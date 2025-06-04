static int verify_sign(void *verify_ctx, uint16_t algo, ptls_iovec_t data, ptls_iovec_t signature)
{
    EVP_PKEY *key = verify_ctx;
    const struct st_ptls_openssl_signature_scheme_t *scheme;
    EVP_MD_CTX *ctx = NULL;
    EVP_PKEY_CTX *pkey_ctx = NULL;
    int ret = 0;

    if (data.base == NULL)
        goto Exit;

    if ((scheme = lookup_signature_schemes(key)) == NULL) {
        ret = PTLS_ERROR_LIBRARY;
        goto Exit;
    }
    for (; scheme->scheme_id != UINT16_MAX; ++scheme)
        if (scheme->scheme_id == algo)
            goto SchemeFound;
    ret = PTLS_ALERT_ILLEGAL_PARAMETER;
    goto Exit;

SchemeFound:
    if ((ctx = EVP_MD_CTX_create()) == NULL) {
        ret = PTLS_ERROR_NO_MEMORY;
        goto Exit;
    }

#if PTLS_OPENSSL_HAVE_ED25519
    if (EVP_PKEY_id(key) == EVP_PKEY_ED25519) {
        /* ED25519 requires the use of the all-at-once function that appeared in OpenSSL 1.1.1, hence different path */
        if (EVP_DigestVerifyInit(ctx, &pkey_ctx, NULL, NULL, key) != 1) {
            ret = PTLS_ERROR_LIBRARY;
            goto Exit;
        }
        if (EVP_DigestVerify(ctx, signature.base, signature.len, data.base, data.len) != 1) {
            ret = PTLS_ERROR_LIBRARY;
            goto Exit;
        }
    } else
#endif
    {
        if (EVP_DigestVerifyInit(ctx, &pkey_ctx, scheme->scheme_md(), NULL, key) != 1) {
            ret = PTLS_ERROR_LIBRARY;
            goto Exit;
        }

        if (EVP_PKEY_id(key) == EVP_PKEY_RSA) {
            if (EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PSS_PADDING) != 1) {
                ret = PTLS_ERROR_LIBRARY;
                goto Exit;
            }
            if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey_ctx, -1) != 1) {
                ret = PTLS_ERROR_LIBRARY;
                goto Exit;
            }
            if (EVP_PKEY_CTX_set_rsa_mgf1_md(pkey_ctx, scheme->scheme_md()) != 1) {
                ret = PTLS_ERROR_LIBRARY;
                goto Exit;
            }
        }
        if (EVP_DigestVerifyUpdate(ctx, data.base, data.len) != 1) {
            ret = PTLS_ERROR_LIBRARY;
            goto Exit;
        }
        if (EVP_DigestVerifyFinal(ctx, signature.base, signature.len) != 1) {
            ret = PTLS_ALERT_DECRYPT_ERROR;
            goto Exit;
        }
    }

    ret = 0;

Exit:
    if (ctx != NULL)
        EVP_MD_CTX_destroy(ctx);
    EVP_PKEY_free(key);
    return ret;
}


// Source: openssl.c
// Lines 1068-1145
