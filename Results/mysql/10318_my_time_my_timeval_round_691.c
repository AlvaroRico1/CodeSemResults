bool my_timeval_round(struct timeval *tv, uint decimals) {
  assert(decimals <= DATETIME_MAX_DECIMALS);
  uint nanoseconds = msec_round_add[decimals];
  tv->tv_usec += (nanoseconds + 500) / 1000;
  if (tv->tv_usec < 1000000) goto ret;

  tv->tv_usec = 0;
  tv->tv_sec++;
  if (!is_time_t_valid_for_timestamp(tv->tv_sec)) {
    tv->tv_sec = TIMESTAMP_MAX_VALUE;
    return true;
  }

ret:
  my_timeval_trunc(tv, decimals);
  return false;
}


// Source: my_time.cc
// Lines 2617-2633
