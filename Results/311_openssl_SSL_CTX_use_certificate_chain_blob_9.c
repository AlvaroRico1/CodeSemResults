// Source: curl/lib/vtls/openssl.c
// Lines 669-677
SSL_CTX_use_certificate_chain_blob(SSL_CTX *ctx, const struct curl_blob *blob,
                                   const char *key_passwd)
{
/* SSL_CTX_add1_chain_cert introduced in OpenSSL 1.0.2 */
#if (OPENSSL_VERSION_NUMBER >= 0x1000200fL) && /* OpenSSL 1.0.2 or later */ \
    !(defined(LIBRESSL_VERSION_NUMBER) && \
      (LIBRESSL_VERSION_NUMBER < 0x2090100fL)) /* LibreSSL 2.9.1 or later */
  int ret = 0;
  X509 *x = NULL;
  void *passwd_callback_userdata = (void *)key_passwd;
  BIO *in = BIO_new_mem_buf(blob->data, (int)(blob->len));
  if(!in)
    return CURLE_OUT_OF_MEMORY;

  ERR_clear_error();

  x = PEM_read_bio_X509_AUX(in, NULL,
                            passwd_callback, (void *)key_passwd);

  if(!x) {
    ret = 0;
    goto end;
  }

  ret = SSL_CTX_use_certificate(ctx, x);

  if(ERR_peek_error() != 0)
    ret = 0;

  if(ret) {
    X509 *ca;
    unsigned long err;

    if(!SSL_CTX_clear_chain_certs(ctx)) {
      ret = 0;
      goto end;
    }

    while((ca = PEM_read_bio_X509(in, NULL, passwd_callback,
                                  passwd_callback_userdata))
          != NULL) {

      if(!SSL_CTX_add0_chain_cert(ctx, ca)) {
        X509_free(ca);
        ret = 0;
        goto end;
      }
    }

    err = ERR_peek_last_error();
    if((ERR_GET_LIB(err) == ERR_LIB_PEM) &&
       (ERR_GET_REASON(err) == PEM_R_NO_START_LINE))
      ERR_clear_error();
    else
      ret = 0;
  }

 end:
  X509_free(x);
  BIO_free(in);
  return ret;
#else
  (void)ctx; /* unused */
  (void)blob; /* unused */
  (void)key_passwd; /* unused */
  return 0;
#endif
}
