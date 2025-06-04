void TIME_from_longlong_datetime_packed(MYSQL_TIME *ltime, longlong tmp) {
  longlong ymd;
  longlong hms;
  longlong ymdhms;
  longlong ym;

  if ((ltime->neg = (tmp < 0))) tmp = -tmp;

  ltime->second_part = my_packed_time_get_frac_part(tmp);
  ymdhms = my_packed_time_get_int_part(tmp);

  ymd = ymdhms >> 17;
  ym = ymd >> 5;
  hms = ymdhms % (1 << 17);

  ltime->day = ymd % (1 << 5);
  ltime->month = ym % 13;
  ltime->year = static_cast<uint>(ym / 13);

  ltime->second = hms % (1 << 6);
  ltime->minute = (hms >> 6) % (1 << 6);
  ltime->hour = static_cast<uint>(hms >> 12);

  ltime->time_type = MYSQL_TIMESTAMP_DATETIME;
  ltime->time_zone_displacement = 0;
}


// Source: my_time.cc
// Lines 1908-1933
