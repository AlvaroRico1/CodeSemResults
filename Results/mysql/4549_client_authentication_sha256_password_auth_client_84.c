int sha256_password_auth_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql) {
  bool uses_password = mysql->passwd[0] != 0;
  unsigned char encrypted_password[MAX_CIPHER_LENGTH];
  static char request_public_key = '\1';
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

  if (mysql_get_ssl_cipher(mysql) != nullptr) connection_is_secure = true;

  /* If connection isn't secure attempt to get the RSA public key file */
  if (!connection_is_secure) {
    public_key = rsa_init(mysql);
  }

  if (!uses_password) {
    /* We're not using a password */
    static const unsigned char zero_byte = '\0';
    if (vio->write_packet(vio, &zero_byte, 1)) return CR_ERROR;
  } else {
    /* Password is a 0-terminated byte array ('\0' character included) */
    unsigned int passwd_len =
        static_cast<unsigned int>(strlen(mysql->passwd) + 1);
    if (!connection_is_secure) {
      /*
        If no public key; request one from the server.
      */
      if (public_key == nullptr) {
        if (vio->write_packet(vio, (const unsigned char *)&request_public_key,
                              1))
          return CR_ERROR;

        int packet_len = 0;
        unsigned char *packet;
        if ((packet_len = vio->read_packet(vio, &packet)) == -1)
          return CR_ERROR;
        BIO *bio = BIO_new_mem_buf(packet, packet_len);
        public_key = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (public_key == nullptr) {
          ERR_clear_error();
          return CR_ERROR;
        }
        got_public_key_from_server = true;
      }

      /*
        An arbitrary limitation based on the assumption that passwords
        larger than e.g. 15 symbols don't contribute to security.
        Note also that it's furter restricted to RSA_size() - 41 down
        below, so this leaves 471 bytes of possible RSA key sizes which
        should be reasonably future-proof.
        We avoid heap allocation for speed reasons.
      */
      char passwd_scramble[512];

      if (passwd_len > sizeof(passwd_scramble)) {
        /* password too long for the buffer */
        if (got_public_key_from_server) RSA_free(public_key);
        return CR_ERROR;
      }
      memmove(passwd_scramble, mysql->passwd, passwd_len);

      /* Obfuscate the plain text password with the session scramble */
      xor_string(passwd_scramble, passwd_len - 1, (char *)scramble_pkt,
                 SCRAMBLE_LENGTH);
      /* Encrypt the password and send it to the server */
      int cipher_length = RSA_size(public_key);
      /*
        When using RSA_PKCS1_OAEP_PADDING the password length must be less
        than RSA_size(rsa) - 41.
      */
      if (passwd_len + 41 >= (unsigned)cipher_length) {
        /* password message is to long */
        if (got_public_key_from_server) RSA_free(public_key);
        return CR_ERROR;
      }
      RSA_public_encrypt(passwd_len, (unsigned char *)passwd_scramble,
                         encrypted_password, public_key,
                         RSA_PKCS1_OAEP_PADDING);
      if (got_public_key_from_server) RSA_free(public_key);

      if (vio->write_packet(vio, (uchar *)encrypted_password, cipher_length))
        return CR_ERROR;
    } else {
      /* The vio is encrypted already; just send the plain text passwd */
      if (vio->write_packet(vio, (uchar *)mysql->passwd, passwd_len))
        return CR_ERROR;
    }
  }

  return CR_OK;
}


// Source: client_authentication.cc
// Lines 141-255
