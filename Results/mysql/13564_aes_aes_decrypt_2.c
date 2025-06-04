aes_return_status aes_decrypt(const unsigned char *source,
                              unsigned int source_length, unsigned char *dest,
                              const unsigned char *key, unsigned int key_length,
                              enum Keyring_aes_opmode mode,
                              const unsigned char *iv, bool padding,
                              size_t *decrypted_length) {
  if (decrypted_length == nullptr) return AES_OUTPUT_SIZE_NULL;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  EVP_CIPHER_CTX stack_ctx;
  EVP_CIPHER_CTX *ctx = &stack_ctx;
#else  /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (ctx == nullptr) return AES_CTX_ALLOCATION_ERROR;
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */

  auto cleanup_guard = create_scope_guard([&] {
    /* need to explicitly clean up the error if we want to ignore it */
    ERR_clear_error();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX_cleanup(ctx);
#else  /* OPENSSL_VERSION_NUMBER < 0x10100000L */
    EVP_CIPHER_CTX_free(ctx);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  });

  const EVP_CIPHER *cipher = aes_evp_type(mode);
  if (cipher == nullptr) return AES_INVALID_BLOCK_MODE;

  /* The real key to be used for encryption */
  std::unique_ptr<unsigned char[]> rkey;
  size_t rkey_size;
  if (aes_create_key(key, key_length, rkey, &rkey_size, mode) == false)
    return AES_KEY_TRANSFORMATION_ERROR;

  if (EVP_CIPHER_iv_length(cipher) > 0 && !iv) return AES_IV_EMPTY;

  int u_len, f_len;

  if (!EVP_DecryptInit(ctx, aes_evp_type(mode), rkey.get(), iv))
    return AES_DECRYPTION_ERROR;
  if (!EVP_CIPHER_CTX_set_padding(ctx, padding)) return AES_DECRYPTION_ERROR;
  if (!EVP_DecryptUpdate(ctx, dest, &u_len, source, source_length))
    return AES_DECRYPTION_ERROR;
  if (!EVP_DecryptFinal_ex(ctx, dest + u_len, &f_len))
    return AES_DECRYPTION_ERROR;

  /* All is well */
  *decrypted_length = static_cast<size_t>(u_len + f_len);
  return AES_OP_OK;
}


// Source: aes.cc
// Lines 190-240
