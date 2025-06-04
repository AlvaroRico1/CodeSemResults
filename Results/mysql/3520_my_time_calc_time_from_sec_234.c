void calc_time_from_sec(MYSQL_TIME *to, longlong seconds, long microseconds) {
  long t_seconds;
  // to->neg is not cleared, it may already be set to a useful value
  to->time_type = MYSQL_TIMESTAMP_TIME;
  to->year = 0;
  to->month = 0;
  to->day = 0;
  assert(seconds < (0xFFFFFFFFLL * 3600LL));
  to->hour = static_cast<long>(seconds / 3600L);
  t_seconds = static_cast<long>(seconds % 3600L);
  to->minute = t_seconds / 60L;
  to->second = t_seconds % 60L;
  to->second_part = microseconds;
}


// Source: my_time.cc
// Lines 2698-2711
