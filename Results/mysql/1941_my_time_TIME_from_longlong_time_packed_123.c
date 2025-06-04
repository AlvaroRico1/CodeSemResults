void TIME_from_longlong_time_packed(MYSQL_TIME *ltime, longlong tmp) {
  longlong hms;
  if ((ltime->neg = (tmp < 0))) tmp = -tmp;
  hms = my_packed_time_get_int_part(tmp);
  ltime->year = static_cast<uint>(0);
  ltime->month = static_cast<uint>(0);
  ltime->day = static_cast<uint>(0);
  ltime->hour =
      static_cast<uint>(hms >> 12) % (1 << 10); /* 10 bits starting at 12th */
  ltime->minute =
      static_cast<uint>(hms >> 6) % (1 << 6); /* 6 bits starting at 6th   */
  ltime->second =
      static_cast<uint>(hms) % (1 << 6); /* 6 bits starting at 0th   */
  ltime->second_part = my_packed_time_get_frac_part(tmp);
  ltime->time_type = MYSQL_TIMESTAMP_TIME;
}


// Source: my_time.cc
// Lines 1713-1728
