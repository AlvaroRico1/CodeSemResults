longlong year_to_longlong_datetime_packed(long year) {
  longlong ymd = ((year * 13) << 5);
  return my_packed_time_make_int(ymd << 17);
}


// Source: my_time.cc
// Lines 1897-1900
