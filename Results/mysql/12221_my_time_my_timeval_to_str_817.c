int my_timeval_to_str(const struct timeval *tm, char *to, uint dec) {
  int len = sprintf(to, "%d", static_cast<int>(tm->tv_sec));
  if (dec) len += my_useconds_to_str(to + len, tm->tv_usec, dec);
  return len;
}


// Source: my_time.cc
// Lines 1427-1431
