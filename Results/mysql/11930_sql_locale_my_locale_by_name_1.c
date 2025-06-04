static MY_LOCALE *my_locale_by_name(MY_LOCALE **locales, const char *name,
                                    size_t length) {
  MY_LOCALE **locale;
  for (locale = locales; *locale != nullptr; locale++) {
    if (length == strlen((*locale)->name) &&
        0 == native_strncasecmp((*locale)->name, name, length))
      return *locale;
  }
  return nullptr;
}


// Source: sql_locale.cc
// Lines 2873-2882
