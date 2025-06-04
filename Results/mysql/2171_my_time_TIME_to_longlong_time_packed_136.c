longlong TIME_to_longlong_time_packed(const MYSQL_TIME &my_time) {
  /* If month is 0, we mix day with hours: "1 00:10:10" -> "24:00:10" */
  long hms = (((my_time.month ? 0 : my_time.day * 24) + my_time.hour) << 12) |
             (my_time.minute << 6) | my_time.second;
  longlong tmp = my_packed_time_make(hms, my_time.second_part);
  return my_time.neg ? -tmp : tmp;
}


// Source: my_time.cc
// Lines 1699-1705
