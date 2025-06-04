static void client_mpvio_info(MYSQL_PLUGIN_VIO *vio,
                              MYSQL_PLUGIN_VIO_INFO *info) {
  MCPVIO_EXT *mpvio = (MCPVIO_EXT *)vio;
  mpvio_info(mpvio->mysql->net.vio, info);
}


// Source: client.cc
// Lines 5202-5206
