bool my_time_adjust_frac(MYSQL_TIME *ltime, uint dec, bool truncate) {
  int warnings = 0;
  assert(dec <= DATETIME_MAX_DECIMALS);
  /* Add half away from zero */
  bool rc = time_add_nanoseconds_adjust_frac(ltime, msec_round_add[dec],
                                             &warnings, truncate);

  /* Truncate non-significant digits */
  my_time_trunc(ltime, dec);
  return rc;
}


// Source: my_time.cc
// Lines 2577-2587
