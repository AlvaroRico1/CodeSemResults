static void read_binary_datetime(MYSQL_TIME *tm, uchar **pos) {
  uint length = net_field_length(pos);

  if (length) {
    uchar *to = *pos;

    tm->neg = false;
    tm->year = (uint)sint2korr(to);
    tm->month = (uint)to[2];
    tm->day = (uint)to[3];

    if (length > 4) {
      tm->hour = (uint)to[4];
      tm->minute = (uint)to[5];
      tm->second = (uint)to[6];
    } else
      tm->hour = tm->minute = tm->second = 0;
    tm->second_part = (length > 7) ? (ulong)sint4korr(to + 7) : 0;
    tm->time_type = MYSQL_TIMESTAMP_DATETIME;

    *pos += length;
  } else


// Source: libmysql.cc
// Lines 2641-2662
