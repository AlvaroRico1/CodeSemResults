static void kill_query(const char *reason) {
  char kill_buffer[40];
  MYSQL *kill_mysql = nullptr;

  kill_mysql = mysql_init(kill_mysql);
  init_connection_options(kill_mysql);

#ifdef HAVE_SETNS
  if (opt_network_namespace && set_network_namespace(opt_network_namespace)) {
    goto err;
  }
#endif

  MYSQL *ret;
  if (dns_srv_name)
    ret = mysql_real_connect_dns_srv(kill_mysql, dns_srv_name, current_user,
                                     opt_password, "", 0);
  else
    ret =
        mysql_real_connect(kill_mysql, current_host, current_user, opt_password,
                           "", opt_mysql_port, opt_mysql_unix_port, 0);
  if (!ret) {
#ifdef HAVE_SETNS
    if (opt_network_namespace) (void)restore_original_network_namespace();
#endif
    tee_fprintf(stdout,
                "%s -- Sorry, cannot connect to the server to kill "
                "query, giving up ...\n",
                reason);
    goto err;
  }

#ifdef HAVE_SETNS
  if (opt_network_namespace && restore_original_network_namespace()) goto err;
#endif

  interrupted_query = true;

  /* mysqld < 5 does not understand KILL QUERY, skip to KILL CONNECTION */
  sprintf(kill_buffer, "KILL %s%lu",
          (mysql_get_server_version(&mysql) < 50000) ? "" : "QUERY ",
          mysql_thread_id(&mysql));

  if (verbose)
    tee_fprintf(stdout, "%s -- sending \"%s\" to server ...\n", reason,
                kill_buffer);
  mysql_real_query(kill_mysql, kill_buffer,
                   static_cast<ulong>(strlen(kill_buffer)));
  tee_fprintf(stdout, "%s -- query aborted\n", reason);

err:
#ifdef HAVE_SETNS
  if (opt_network_namespace) (void)release_network_namespace_resources();
#endif
  mysql_close(kill_mysql);

  return;
}


// Source: mysql.cc
// Lines 1569-1626
