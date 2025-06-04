int caching_sha2_password_auth_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql) {
  bool uses_password = mysql->passwd[0] != 0;
  unsigned char encrypted_password[MAX_CIPHER_LENGTH];
  // static char request_public_key= '\1';
  RSA *public_key = nullptr;
  bool got_public_key_from_server = false;
  bool connection_is_secure = false;
  unsigned char scramble_pkt[20];
  unsigned char *pkt;

  DBUG_TRACE;

  /*
    Get the scramble from the server because we need it when sending encrypted
    password.
  */
  if (vio->read_packet(vio, &pkt) != SCRAMBLE_LENGTH + 1) {
    DBUG_PRINT("info", ("Scramble is not of correct length."));
    return CR_ERROR;
  }
  if (pkt[SCRAMBLE_LENGTH] != '\0') {
    DBUG_PRINT("info", ("Missing protocol token in scramble data."));
    return CR_ERROR;
  }

  /*
    Copy the scramble to the stack or it will be lost on the next use of the
    net buffer.
  */
  memcpy(scramble_pkt, pkt, SCRAMBLE_LENGTH);

  connection_is_secure = is_secure_transport(mysql);

  if (!uses_password) {
    /* We're not using a password */
    static const unsigned char zero_byte = '\0';
    if (vio->write_packet(vio, &zero_byte, 1)) return CR_ERROR;
    return CR_OK;
  } else {
    /* Password is a 0-terminated byte array ('\0' character included) */
    unsigned int passwd_len =
        static_cast<unsigned int>(strlen(mysql->passwd) + 1);
    int pkt_len = 0;
    {
      /* First try with SHA2 scramble */
      unsigned char sha2_scramble[SHA2_SCRAMBLE_LENGTH];
      if (generate_sha256_scramble(sha2_scramble, SHA2_SCRAMBLE_LENGTH,
                                   mysql->passwd, passwd_len - 1,
                                   (char *)scramble_pkt, SCRAMBLE_LENGTH)) {
        set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_ERR, unknown_sqlstate,
                                 ER_CLIENT(CR_AUTH_PLUGIN_ERR),
                                 "caching_sha2_password",
                                 "Failed to generate scramble");
        return CR_ERROR;
      }

      if (vio->write_packet(vio, sha2_scramble, SHA2_SCRAMBLE_LENGTH))
        return CR_ERROR;

      if ((pkt_len = vio->read_packet(vio, &pkt)) == -1) return CR_ERROR;

      if (pkt_len == 1 && *pkt == fast_auth_success) return CR_OK;

      /* An OK packet would follow */
    }

    if (pkt_len != 1 || *pkt != perform_full_authentication) {
      DBUG_PRINT("info", ("Unexpected reply from server."));
      return CR_ERROR;
    }

    /* If connection isn't secure attempt to get the RSA public key file */
    if (!connection_is_secure) {
      public_key = rsa_init(mysql);

      if (public_key == nullptr && mysql->options.extension &&
          mysql->options.extension->get_server_public_key) {
        // If no public key; request one from the server.
        if (vio->write_packet(vio, (const unsigned char *)&request_public_key,
                              1))
          return CR_ERROR;

        if ((pkt_len = vio->read_packet(vio, &pkt)) <= 0) return CR_ERROR;
        BIO *bio = BIO_new_mem_buf(pkt, pkt_len);
        public_key = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (public_key == nullptr) {
          ERR_clear_error();
          DBUG_PRINT("info", ("Failed to parse public key"));
          return CR_ERROR;
        }
        got_public_key_from_server = true;
      }


// Source: client_authentication.cc
// Lines 447-539
