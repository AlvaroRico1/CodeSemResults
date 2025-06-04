static int set_connect_attributes(MYSQL *mysql, char *buff, size_t buf_len) {
  DBUG_TRACE;
  int rc = 0;

  /*
    Clean up any values set by the client code. We want these options as
    consistent as possible
  */
  rc += mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, "_client_name");
  rc += mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, "_os");
  rc += mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, "_platform");
  rc += mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, "_pid");
  rc += mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, "_thread");
  rc += mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_DELETE, "_client_version");

  /*
   Now let's set up some values
  */
  rc += mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_client_name",
                       "libmysql");
  rc += mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_client_version",
                       PACKAGE_VERSION);
  rc += mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_os", SYSTEM_TYPE);
  rc += mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_platform",
                       MACHINE_TYPE);
#ifdef _WIN32
  snprintf(buff, buf_len, "%lu", (ulong)GetCurrentProcessId());
#else
  snprintf(buff, buf_len, "%lu", (ulong)getpid());
#endif
  rc += mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_pid", buff);

#ifdef _WIN32
  snprintf(buff, buf_len, "%lu", (ulong)GetCurrentThreadId());
  rc += mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_thread", buff);
#endif

  return rc > 0 ? 1 : 0;
}


// Source: client.cc
// Lines 5582-5620
