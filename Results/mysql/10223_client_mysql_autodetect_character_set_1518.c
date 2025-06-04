static int mysql_autodetect_character_set(MYSQL *mysql) {
  const char *csname = MYSQL_DEFAULT_CHARSET_NAME;

#ifdef _WIN32
  char cpbuf[64];
  {
    snprintf(cpbuf, sizeof(cpbuf), "cp%d", (int)GetConsoleCP());
    csname = my_os_charset_to_mysql_charset(cpbuf);
  }
#elif defined(HAVE_NL_LANGINFO)
  {
    if (setlocale(LC_CTYPE, "") && (csname = nl_langinfo(CODESET)))
      csname = my_os_charset_to_mysql_charset(csname);
  }
#endif

  if (mysql->options.charset_name) my_free(mysql->options.charset_name);
  if (!(mysql->options.charset_name =
            my_strdup(key_memory_mysql_options, csname, MYF(MY_WME))))
    return 1;
  return 0;
}


// Source: client.cc
// Lines 3682-3703
