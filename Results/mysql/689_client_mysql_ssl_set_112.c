bool STDCALL mysql_ssl_set(MYSQL *mysql MY_ATTRIBUTE((unused)),
                           const char *key MY_ATTRIBUTE((unused)),
                           const char *cert MY_ATTRIBUTE((unused)),
                           const char *ca MY_ATTRIBUTE((unused)),
                           const char *capath MY_ATTRIBUTE((unused)),
                           const char *cipher MY_ATTRIBUTE((unused))) {
  bool result = false;
  DBUG_TRACE;
  result = mysql_options(mysql, MYSQL_OPT_SSL_KEY, key) +
                   mysql_options(mysql, MYSQL_OPT_SSL_CERT, cert) +
                   mysql_options(mysql, MYSQL_OPT_SSL_CA, ca) +
                   mysql_options(mysql, MYSQL_OPT_SSL_CAPATH, capath) +
                   mysql_options(mysql, MYSQL_OPT_SSL_CIPHER, cipher)
               ? true
               : false;
  return result;
}


// Source: client.cc
// Lines 3278-3294
