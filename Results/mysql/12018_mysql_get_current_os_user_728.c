static void get_current_os_user() {
  const char *user;

#ifdef _WIN32
  char buf[255];
  WCHAR wbuf[255];
  DWORD wbuf_len = sizeof(wbuf) / sizeof(WCHAR);
  size_t len;
  uint dummy_errors;

  if (GetUserNameW(wbuf, &wbuf_len)) {
    len = my_convert(buf, sizeof(buf) - 1, charset_info, (char *)wbuf,
                     wbuf_len * sizeof(WCHAR), &my_charset_utf16le_bin,
                     &dummy_errors);
    buf[len] = 0;
    user = buf;
  } else {
    user = "UNKNOWN USER";
  }
#else
#ifdef HAVE_GETPWUID
  struct passwd *pw;

  if ((pw = getpwuid(geteuid())) != nullptr)
    user = pw->pw_name;
  else
#endif
      if (!(user = getenv("USER")) && !(user = getenv("LOGNAME")) &&
          !(user = getenv("LOGIN")))
    user = "UNKNOWN USER";
#endif /* _WIN32 */
  current_os_user = my_strdup(PSI_NOT_INSTRUMENTED, user, MYF(MY_WME));
  return;
}


// Source: mysql.cc
// Lines 5275-5308
