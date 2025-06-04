MYSQL *STDCALL mysql_real_connect_dns_srv(MYSQL *mysql,
                                          const char *dns_srv_name,
                                          const char *user, const char *passwd,
                                          const char *db,
                                          unsigned long client_flag) {
  Dns_srv_data data;
  int err = 0;

  if (get_dns_srv(data, dns_srv_name, err)) {
    set_mysql_extended_error(mysql, CR_DNS_SRV_LOOKUP_FAILED, unknown_sqlstate,
                             ER_CLIENT(CR_DNS_SRV_LOOKUP_FAILED), err);
    return nullptr;
  }

  std::string host;
  uint port;
  while (!data.pop_next(host, port)) {
    MYSQL *ret =
        mysql_real_connect(mysql, host.c_str(), user, passwd, db, port, NULL,
                           client_flag | CLIENT_REMEMBER_OPTIONS);
    if (ret) return ret;
  }


// Source: dns_srv.cc
// Lines 161-182
