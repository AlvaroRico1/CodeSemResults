static int clear_password_auth_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql) {
  int res;

  /* send password in clear text */
  res = vio->write_packet(vio, (const unsigned char *)mysql->passwd,
                          (int)strlen(mysql->passwd) + 1);

  return res ? CR_ERROR : CR_OK;
}


// Source: client.cc
// Lines 8828-8836
