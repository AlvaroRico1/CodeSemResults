int my_aes_encrypt(const unsigned char *source, uint32 source_length,
                   unsigned char *dest, const unsigned char *key,
                   uint32 key_length, enum my_aes_opmode mode,
                   const unsigned char *iv, bool padding) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  EVP_CIPHER_CTX stack_ctx;
  EVP_CIPHER_CTX *ctx = &stack_ctx;
#else  /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  const EVP_CIPHER *cipher = aes_evp_type(mode);
  int u_len, f_len;
  /* The real key to be used for encryption */
  unsigned char rkey[MAX_AES_KEY_LENGTH / 8];
  my_aes_create_key(key, key_length, rkey, mode);

  if (!ctx || !cipher || (EVP_CIPHER_iv_length(cipher) > 0 && !iv))
    return MY_AES_BAD_DATA;

  if (!EVP_EncryptInit(ctx, cipher, rkey, iv)) goto aes_error;   /* Error */
  if (!EVP_CIPHER_CTX_set_padding(ctx, padding)) goto aes_error; /* Error */
  if (!EVP_EncryptUpdate(ctx, dest, &u_len, source, source_length))
    goto aes_error; /* Error */

  if (!EVP_EncryptFinal(ctx, dest + u_len, &f_len)) goto aes_error; /* Error */

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  EVP_CIPHER_CTX_cleanup(ctx);
#else  /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  EVP_CIPHER_CTX_free(ctx);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  return u_len + f_len;

aes_error:
  /* need to explicitly clean up the error if we want to ignore it */
  ERR_clear_error();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  EVP_CIPHER_CTX_cleanup(ctx);
#else  /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  EVP_CIPHER_CTX_free(ctx);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  return MY_AES_BAD_DATA;
}


// Source: my_aes_openssl.cc
// Lines 116-158
