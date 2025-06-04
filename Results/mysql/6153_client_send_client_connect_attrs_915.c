uchar *send_client_connect_attrs(MYSQL *mysql, uchar *buf) {
  /* check if the server supports connection attributes */
  if (mysql->server_capabilities & CLIENT_CONNECT_ATTRS) {
    /* Always store the length if the client supports it */
    buf = net_store_length(
        buf, mysql->options.extension
                 ? mysql->options.extension->connection_attributes_length
                 : 0);

    /* check if we have connection attributes */
    if (mysql->options.extension &&
        mysql->options.extension->connection_attributes) {
      /* loop over and dump the connection attributes */
      for (const auto &key_and_value :
           mysql->options.extension->connection_attributes->hash) {
        const string &key = key_and_value.first;
        const string &value = key_and_value.second;

        /* we can't have zero length keys */
        assert(!key.empty());

        buf = write_length_encoded_string3(buf, key.data(), key.size());
        buf = write_length_encoded_string3(buf, value.data(), value.size());
      }
    }


// Source: client.cc
// Lines 3879-3903
