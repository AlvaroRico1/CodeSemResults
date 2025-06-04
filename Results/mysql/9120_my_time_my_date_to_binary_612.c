void my_date_to_binary(const MYSQL_TIME *ltime, uchar *ptr) {
  long tmp = ltime->day + ltime->month * 32 + ltime->year * 16 * 32;
  int3store(ptr, tmp);
}


// Source: my_time.cc
// Lines 2086-2089
