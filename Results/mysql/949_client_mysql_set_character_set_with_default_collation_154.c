static void mysql_set_character_set_with_default_collation(MYSQL *mysql) {
  const char *save = charsets_dir;
  if (mysql->options.charset_dir) {
#ifdef MYSQL_SERVER
    // Do not change charsets_dir, it is not thread safe.
    assert(false);
#else
    charsets_dir = mysql->options.charset_dir;
#endif
  }
  if ((mysql->charset = get_charset_by_csname(mysql->options.charset_name,
                                              MY_CS_PRIMARY, MYF(MY_WME)))) {
    /* Try to set compiled default collation when it's possible. */
    CHARSET_INFO *collation;
    if ((collation =
             get_charset_by_name(MYSQL_DEFAULT_COLLATION_NAME, MYF(MY_WME))) &&
        my_charset_same(mysql->charset, collation)) {
      mysql->charset = collation;
    } else {
      /*
        Default compiled collation not found, or is not applicable
        to the requested character set.
        Continue with the default collation of the character set.
      */
    }
  }


// Source: client.cc
// Lines 3705-3730
