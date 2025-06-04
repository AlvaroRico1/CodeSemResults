int my_date_to_str(const MYSQL_TIME &my_time, char *to) {
  const char *const start = to;
  to = format_two_digits(my_time.year / 100, to);
  to = format_two_digits(my_time.year % 100, to);
  *to++ = '-';
  to = format_two_digits(my_time.month, to);
  *to++ = '-';
  to = format_two_digits(my_time.day, to);
  *to = '\0';
  return to - start;
}


// Source: my_time.cc
// Lines 1323-1333
