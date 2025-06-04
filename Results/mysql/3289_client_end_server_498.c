void end_server(MYSQL *mysql) {
  int save_errno = errno;
  DBUG_TRACE;
  if (mysql->net.vio != nullptr) {
#ifndef NDEBUG
    char desc[VIO_DESCRIPTION_SIZE];
    vio_description(mysql->net.vio, desc);
    DBUG_PRINT("info", ("Net: %s", desc));
#endif  // NDEBUG
#ifdef MYSQL_SERVER
    slave_io_thread_detach_vio();
#endif
    vio_delete(mysql->net.vio);
    mysql->net.vio = nullptr; /* Marker */
    mysql_prune_stmt_list(mysql);
  }
  net_end(&mysql->net);
  //  net_extension_free(&mysql->net);
  free_old_query(mysql);
  errno = save_errno;
  MYSQL_TRACE(DISCONNECTED, mysql, ());
}


// Source: client.cc
// Lines 1828-1849
