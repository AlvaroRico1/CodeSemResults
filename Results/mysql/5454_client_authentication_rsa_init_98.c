static RSA *rsa_init(MYSQL *mysql) {
  RSA *key = nullptr;

  mysql_mutex_lock(&g_public_key_mutex);
  key = g_public_key;
  mysql_mutex_unlock(&g_public_key_mutex);

  if (key != nullptr) return key;

  FILE *pub_key_file = nullptr;

  if (mysql->options.extension != nullptr &&
      mysql->options.extension->server_public_key_path != nullptr &&
      mysql->options.extension->server_public_key_path[0] != '\0') {
    pub_key_file =
        fopen(mysql->options.extension->server_public_key_path, "rb");
  }
  /* No public key is used; return 0 without errors to indicate this. */
  else
    return nullptr;

  if (pub_key_file == nullptr) {
    /*
      If a key path was submitted but no key located then we print an error
      message. Else we just report that there is no public key.
    */
    my_message_local(WARNING_LEVEL, EE_FAILED_TO_LOCATE_SERVER_PUBLIC_KEY,
                     mysql->options.extension->server_public_key_path);

    return nullptr;
  }

  mysql_mutex_lock(&g_public_key_mutex);
  key = g_public_key =
      PEM_read_RSA_PUBKEY(pub_key_file, nullptr, nullptr, nullptr);
  mysql_mutex_unlock(&g_public_key_mutex);
  fclose(pub_key_file);
  if (g_public_key == nullptr) {
    ERR_clear_error();
    my_message_local(WARNING_LEVEL, EE_PUBLIC_KEY_NOT_IN_PEM_FORMAT,
                     mysql->options.extension->server_public_key_path);
    return nullptr;
  }

  return key;
}


// Source: client_authentication.cc
// Lines 83-128
