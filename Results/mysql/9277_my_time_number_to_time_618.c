bool number_to_time(longlong nr, MYSQL_TIME *ltime, int *warnings) {
  if (nr > TIME_MAX_VALUE) {
    /* For huge numbers try full DATETIME, like str_to_time does. */
    if (nr >= 10000000000LL) /* '0001-00-00 00-00-00' */
    {
      int warnings_backup = *warnings;
      if (number_to_datetime(nr, ltime, 0, warnings) != -1LL) return false;
      *warnings = warnings_backup;
    }


// Source: my_time.cc
// Lines 947-955
