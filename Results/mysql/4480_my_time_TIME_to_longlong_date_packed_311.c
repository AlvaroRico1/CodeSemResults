longlong TIME_to_longlong_date_packed(const MYSQL_TIME &my_time) {
  longlong ymd = ((my_time.year * 13 + my_time.month) << 5) | my_time.day;
  return my_packed_time_make_int(ymd << 17);
}


// Source: my_time.cc
// Lines 1886-1889
