static mysql_compress_context *compress_context(NET *net) {
  mysql_compress_context *mysql_compress_ctx = nullptr;
#ifdef MYSQL_SERVER
  NET_SERVER *server_extension = static_cast<NET_SERVER *>(net->extension);
  if (server_extension != nullptr) {
    mysql_compress_ctx = &server_extension->compress_ctx;
  }
#else
  NET_EXTENSION *ext = NET_EXTENSION_PTR(net);
  if (ext != nullptr) mysql_compress_ctx = &ext->compress_ctx;
#endif
  return mysql_compress_ctx;
}


// Source: net_serv.cc
// Lines 129-141
