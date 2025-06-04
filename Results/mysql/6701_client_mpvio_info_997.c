void mpvio_info(Vio *vio, MYSQL_PLUGIN_VIO_INFO *info) {
  memset(info, 0, sizeof(*info));
  switch (vio->type) {
    case VIO_TYPE_TCPIP:
      info->protocol = MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_TCP;
      info->socket = (int)vio_fd(vio);
      return;
    case VIO_TYPE_SOCKET:
      info->protocol = MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_SOCKET;
      info->socket = (int)vio_fd(vio);
      return;
    case VIO_TYPE_SSL: {
      struct sockaddr addr;
      socklen_t addrlen = sizeof(addr);
      if (getsockname(vio_fd(vio), &addr, &addrlen)) return;
      info->protocol = addr.sa_family == AF_UNIX
                           ? MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_SOCKET
                           : MYSQL_PLUGIN_VIO_INFO::MYSQL_VIO_TCP;
      info->socket = (int)vio_fd(vio);
      return;
    }


// Source: client.cc
// Lines 5164-5184
