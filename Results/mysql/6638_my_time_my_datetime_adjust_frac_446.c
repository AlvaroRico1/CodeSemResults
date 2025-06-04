bool my_datetime_adjust_frac(MYSQL_TIME *ltime, uint dec, int *warnings,
                             bool truncate) {
  assert(dec <= DATETIME_MAX_DECIMALS);
  /* Add half away from zero */
  bool rc = datetime_add_nanoseconds_adjust_frac(ltime, msec_round_add[dec],
                                                 warnings, truncate);
  /* Truncate non-significant digits */
  my_time_trunc(ltime, dec);
  return rc;
}


// Source: my_time.cc
// Lines 2599-2608
