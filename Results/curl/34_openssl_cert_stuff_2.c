int cert_stuff(struct Curl_easy *data,
               SSL_CTX* ctx,
               char *cert_file,
               const struct curl_blob *cert_blob,
               const char *cert_type,
               char *key_file,
               const struct curl_blob *key_blob,
               const char *key_type,
               char *key_passwd)
{
  char error_buffer[256];
  bool check_privkey = TRUE;

  int file_type = do_file_type(cert_type);

  if(cert_file || cert_blob || (file_type == SSL_FILETYPE_ENGINE)) {
    SSL *ssl;
    X509 *x509;
    int cert_done = 0;
    int cert_use_result;

    if(key_passwd) {
      /* set the password in the callback userdata */
      SSL_CTX_set_default_passwd_cb_userdata(ctx, key_passwd);
      /* Set passwd callback: */
      SSL_CTX_set_default_passwd_cb(ctx, passwd_callback);
    }


    switch(file_type) {
    case SSL_FILETYPE_PEM:
      /* SSL_CTX_use_certificate_chain_file() only works on PEM files */
      cert_use_result = cert_blob ?
        SSL_CTX_use_certificate_chain_blob(ctx, cert_blob, key_passwd) :
        SSL_CTX_use_certificate_chain_file(ctx, cert_file);
      if(cert_use_result != 1) {
        failf(data,
              "could not load PEM client certificate, " OSSL_PACKAGE
              " error %s, "
              "(no key found, wrong pass phrase, or wrong file format?)",
              ossl_strerror(ERR_get_error(), error_buffer,
                            sizeof(error_buffer)) );
        return 0;
      }
      break;

    case SSL_FILETYPE_ASN1:
      /* SSL_CTX_use_certificate_file() works with either PEM or ASN1, but
         we use the case above for PEM so this can only be performed with
         ASN1 files. */

      cert_use_result = cert_blob ?
        SSL_CTX_use_certificate_blob(ctx, cert_blob,
                                     file_type, key_passwd) :
        SSL_CTX_use_certificate_file(ctx, cert_file, file_type);
      if(cert_use_result != 1) {
        failf(data,
              "could not load ASN1 client certificate, " OSSL_PACKAGE
              " error %s, "
              "(no key found, wrong pass phrase, or wrong file format?)",
              ossl_strerror(ERR_get_error(), error_buffer,
                            sizeof(error_buffer)) );
        return 0;
      }
      break;
    case SSL_FILETYPE_ENGINE:
#if defined(USE_OPENSSL_ENGINE) && defined(ENGINE_CTRL_GET_CMD_FROM_NAME)
      {
        /* Implicitly use pkcs11 engine if none was provided and the
         * cert_file is a PKCS#11 URI */
        if(!data->state.engine) {
          if(is_pkcs11_uri(cert_file)) {
            if(ossl_set_engine(data, "pkcs11") != CURLE_OK) {
              return 0;
            }
          }
        }

        if(data->state.engine) {
          const char *cmd_name = "LOAD_CERT_CTRL";
          struct {
            const char *cert_id;
            X509 *cert;
          } params;


// Source: openssl.c
// Lines 738-821
