void net_extension_free(NET *net) {
  NET_EXTENSION *ext = NET_EXTENSION_PTR(net);
  if (ext) {
#ifndef MYSQL_SERVER
    if (ext->net_async_context) {
      my_free(ext->net_async_context);
      ext->net_async_context = nullptr;
    }
    mysql_compress_context_deinit(&ext->compress_ctx);
    my_free(ext);
    net->extension = nullptr;
#endif
  }
}


// Source: net_serv.cc
// Lines 104-117
