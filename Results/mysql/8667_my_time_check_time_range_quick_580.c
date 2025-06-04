bool check_time_range_quick(const MYSQL_TIME &my_time) {
  longlong hour = static_cast<longlong>(my_time.hour) + 24LL * my_time.day;
  /* The input value should not be fatally bad */
  assert(!check_time_mmssff_range(my_time));
  if (hour <= TIME_MAX_HOUR &&
      (hour != TIME_MAX_HOUR || my_time.minute != TIME_MAX_MINUTE ||
       my_time.second != TIME_MAX_SECOND || !my_time.second_part))
    return false;
  return true;
}


// Source: my_time.cc
// Lines 240-249
