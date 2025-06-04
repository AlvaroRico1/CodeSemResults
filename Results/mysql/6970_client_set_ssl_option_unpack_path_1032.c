static char *set_ssl_option_unpack_path(const char *arg) {
  char *opt_var = nullptr;
  if (arg) {
    char *buff =
        (char *)my_malloc(key_memory_mysql_options, FN_REFLEN + 1, MYF(MY_WME));
    unpack_filename(buff, arg);
    opt_var = my_strdup(key_memory_mysql_options, buff, MYF(MY_WME));
    my_free(buff);
  }


// Source: client.cc
// Lines 2033-2041
