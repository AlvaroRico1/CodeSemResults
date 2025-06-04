static mysql_state_machine_status csm_prep_select_database(
    mysql_async_connect *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  NET *net = &mysql->net;

  MYSQL_TRACE_STAGE(mysql, READY_FOR_COMMAND);

  /* We will use compression */
  if ((mysql->client_flag & CLIENT_COMPRESS) ||
      (mysql->client_flag & CLIENT_ZSTD_COMPRESSION_ALGORITHM)) {
    net->compress = true;
    uint compress_level;
    enum enum_compression_algorithm algorithm =
        mysql->client_flag & CLIENT_COMPRESS ? MYSQL_ZLIB : MYSQL_ZSTD;
    if (mysql->options.extension &&
        mysql->options.extension->zstd_compression_level)
      compress_level = mysql->options.extension->zstd_compression_level;
    else
      compress_level = mysql_default_compression_level(algorithm);
#ifndef MYSQL_SERVER
    NET_EXTENSION *ext = NET_EXTENSION_PTR(net);
    assert(ext != nullptr);
    mysql_compress_context_init(&ext->compress_ctx, algorithm, compress_level);
#else
    NET_SERVER *server_ext = static_cast<NET_SERVER *>(net->extension);
    if (server_ext == nullptr) {
      server_ext =
          static_cast<NET_SERVER *>(MYSQL_EXTENSION_PTR(mysql)->server_extn);
      net->extension = server_ext;
    }
    assert(server_ext != nullptr);
    mysql_compress_context_init(&server_ext->compress_ctx, algorithm,
                                compress_level);
#endif
  }


// Source: client.cc
// Lines 6481-6516
