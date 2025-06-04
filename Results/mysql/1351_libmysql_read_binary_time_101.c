static void read_binary_time(MYSQL_TIME *tm, uchar **pos) {
  /* net_field_length will set pos to the first byte of data */
  uint length = net_field_length(pos);

  if (length) {
    uchar *to = *pos;
    tm->neg = to[0];

    tm->day = (ulong)sint4korr(to + 1);
    tm->hour = (uint)to[5];
    tm->minute = (uint)to[6];
    tm->second = (uint)to[7];
    tm->second_part = (length > 8) ? (ulong)sint4korr(to + 8) : 0;
    tm->year = tm->month = 0;
    if (tm->day) {
      /* Convert days to hours at once */
      tm->hour += tm->day * 24;
      tm->day = 0;
    }
    tm->time_type = MYSQL_TIMESTAMP_TIME;

    *pos += length;
  } else


// Source: libmysql.cc
// Lines 2615-2637
