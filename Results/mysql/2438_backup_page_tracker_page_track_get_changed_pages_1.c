long long Backup_page_tracker::page_track_get_changed_pages(UDF_INIT *,
                                                            UDF_ARGS *args,
                                                            unsigned char *,
                                                            unsigned char *) {
  MYSQL_THD thd;
  if (mysql_service_mysql_current_thread_reader->get(&thd)) {
    return (-1);
  }

  if (args->arg_count != 2 || args->arg_type[0] != INT_RESULT ||
      args->arg_type[1] != INT_RESULT) {
    return (-1);
  }

  if (!mysqlbackup_backup_id) {
    return (-1);
  }
  // Not expecting anything other than digits in the backupid.
  // Make sure no elements of a relative path are there if the
  // above rule is relaxed
  std::string backupid = mysqlbackup_backup_id;
  if (!std::all_of(backupid.begin(), backupid.end(), ::isdigit)) return 1;

  char mysqlbackup_backupdir[1023];
  void *p = &mysqlbackup_backupdir;
  size_t var_len = 1023;

  mysql_service_component_sys_variable_register->get_variable(
      "mysql_server", "datadir", (void **)&p, &var_len);
  if (var_len == 0) return 2;

  std::string changed_pages_file_dir =
      mysqlbackup_backupdir + Backup_comp_constants::backup_scratch_dir;

#if defined _MSC_VER
  _mkdir(changed_pages_file_dir.c_str());
#else
  if (!mkdir(changed_pages_file_dir.c_str(), 0777)) {
    // no error if already exists
  }
#endif

  changed_pages_file = changed_pages_file_dir + FN_LIBCHAR + backupid +
                       Backup_comp_constants::change_file_extension;
  // if file already exists return error
  FILE *fd = fopen(changed_pages_file.c_str(), "r");
  if (fd) {
    fclose(fd);
    return (-1);
  }

  // get the values form the agrs passed to UDF
  uint64_t start_lsn = *((long long *)args->args[0]);
  uint64_t stop_lsn = *((long long *)args->args[1]);

  Backup_page_tracker::m_receive_changed_page_data = true;
  int status = mysql_service_mysql_page_track->get_page_ids(
      thd, PAGE_TRACK_SE_INNODB, &start_lsn, &stop_lsn,
      Backup_page_tracker::m_changed_pages_buf, CHANGED_PAGES_BUFFER_SIZE,
      page_track_callback, nullptr);
  Backup_page_tracker::m_receive_changed_page_data = false;

  return (status);
}


// Source: backup_page_tracker.cc
// Lines 342-405
