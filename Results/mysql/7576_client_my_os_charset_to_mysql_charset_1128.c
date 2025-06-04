const char *my_os_charset_to_mysql_charset(const char *csname) {
  const MY_CSET_OS_NAME *csp;
  for (csp = charsets; csp->os_name; csp++) {
    if (!my_strcasecmp(&my_charset_latin1, csp->os_name, csname)) {
      switch (csp->param) {
        case my_cs_exact:
          return csp->my_name;

        case my_cs_approx:
          /*
            Maybe we should print a warning eventually:
            character set correspondence is not exact.
          */
          return csp->my_name;

        default:
          my_printf_error(ER_UNKNOWN_ERROR,
                          "OS character set '%s'"
                          " is not supported by MySQL client",
                          MYF(0), csp->my_name);
          goto def;
      }
    }
  }

  my_printf_error(ER_UNKNOWN_ERROR, "Unknown OS character set '%s'.", MYF(0),
                  csname);

def:
  csname = MYSQL_DEFAULT_CHARSET_NAME;
  my_printf_error(ER_UNKNOWN_ERROR,
                  "Switching to the default character set '%s'.", MYF(0),
                  csname);
  return csname;
}


// Source: client.cc
// Lines 3638-3672
