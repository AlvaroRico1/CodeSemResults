int my_time_to_str(const MYSQL_TIME &my_time, char *to, uint dec) {
  const char *const start = to;
  if (my_time.neg) *to++ = '-';

  // Hours should be zero-padded up to two digits. It might have more digits.
  to = write_digits(my_time.hour, std::max(2, count_digits(my_time.hour)), to);

  *to++ = ':';
  to = format_two_digits(my_time.minute, to);
  *to++ = ':';
  to = format_two_digits(my_time.second, to);

  const int length = to - start;
  if (dec) return length + my_useconds_to_str(to, my_time.second_part, dec);
  *to = '\0';
  return length;
}


// Source: my_time.cc
// Lines 1293-1309
