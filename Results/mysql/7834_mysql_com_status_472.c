static int com_status(String *buffer MY_ATTRIBUTE((unused)),
                      char *line MY_ATTRIBUTE((unused))) {
  const char *status_str;
  char buff[40];
  ulonglong id;
  MYSQL_RES *result = nullptr;

  if (mysql_real_query_for_lazy(
          STRING_WITH_LEN("select DATABASE(), USER() limit 1")))
    return 0;

  tee_puts("--------------", stdout);
  usage(1); /* Print version */
  tee_fprintf(stdout, "\nConnection id:\t\t%lu\n", mysql_thread_id(&mysql));
  /*
    Don't remove "limit 1",
    it is protection againts SQL_SELECT_LIMIT=0
  */
  if (!mysql_store_result_for_lazy(&result)) {
    MYSQL_ROW cur = mysql_fetch_row(result);
    if (cur) {
      tee_fprintf(stdout, "Current database:\t%s\n", cur[0] ? cur[0] : "");
      tee_fprintf(stdout, "Current user:\t\t%s\n", cur[1]);
    }
    mysql_free_result(result);
  }

  if ((status_str = mysql_get_ssl_cipher(&mysql)))
    tee_fprintf(stdout, "SSL:\t\t\tCipher in use is %s\n", status_str);
  else
    tee_puts("SSL:\t\t\tNot in use", stdout);

  if (skip_updates) {
    tee_fprintf(stdout, "\nAll updates ignored to this database\n");
  }
#ifdef USE_POPEN
  tee_fprintf(stdout, "Current pager:\t\t%s\n", pager);
  tee_fprintf(stdout, "Using outfile:\t\t'%s'\n", opt_outfile ? outfile : "");
#endif
  tee_fprintf(stdout, "Using delimiter:\t%s\n", delimiter);
  tee_fprintf(stdout, "Server version:\t\t%s\n", server_version_string(&mysql));
  tee_fprintf(stdout, "Protocol version:\t%d\n", mysql_get_proto_info(&mysql));
  tee_fprintf(stdout, "Connection:\t\t%s\n", mysql_get_host_info(&mysql));
  if ((id = mysql_insert_id(&mysql)))
    tee_fprintf(stdout, "Insert id:\t\t%s\n", llstr(id, buff));

  /* "limit 1" is protection against SQL_SELECT_LIMIT=0 */
  if (mysql_real_query_for_lazy(STRING_WITH_LEN(
          "select @@character_set_client, @@character_set_connection, "
          "@@character_set_server, @@character_set_database limit 1"))) {
    if (mysql_errno(&mysql) == CR_SERVER_GONE_ERROR) return 0;
  }
  if (!mysql_store_result_for_lazy(&result)) {
    MYSQL_ROW cur = mysql_fetch_row(result);
    if (cur) {
      tee_fprintf(stdout, "Server characterset:\t%s\n", cur[2] ? cur[2] : "");
      tee_fprintf(stdout, "Db     characterset:\t%s\n", cur[3] ? cur[3] : "");
      tee_fprintf(stdout, "Client characterset:\t%s\n", cur[0] ? cur[0] : "");
      tee_fprintf(stdout, "Conn.  characterset:\t%s\n", cur[1] ? cur[1] : "");
    }
    mysql_free_result(result);
  } else {
    /* Probably pre-4.1 server */
    tee_fprintf(stdout, "Client characterset:\t%s\n",
                replace_utf8_utf8mb3(charset_info->csname));
    tee_fprintf(stdout, "Server characterset:\t%s\n",
                replace_utf8_utf8mb3(mysql.charset->csname));
  }


// Source: mysql.cc
// Lines 4731-4798
