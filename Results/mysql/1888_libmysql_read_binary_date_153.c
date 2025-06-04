static void read_binary_date(MYSQL_TIME *tm, uchar **pos) {
  uint length = net_field_length(pos);

  if (length) {
    uchar *to = *pos;
    tm->year = (uint)sint2korr(to);
    tm->month = (uint)to[2];
    tm->day = (uint)to[3];

    tm->hour = tm->minute = tm->second = 0;
    tm->second_part = 0;
    tm->neg = false;
    tm->time_type = MYSQL_TIMESTAMP_DATE;

    *pos += length;
  } else


// Source: libmysql.cc
// Lines 2666-2681
