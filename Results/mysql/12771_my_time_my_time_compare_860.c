int my_time_compare(const MYSQL_TIME &my_time_a, const MYSQL_TIME &my_time_b) {
  ulonglong a_t = TIME_to_ulonglong_datetime(my_time_a);
  ulonglong b_t = TIME_to_ulonglong_datetime(my_time_b);

  if (a_t < b_t) return -1;
  if (a_t > b_t) return 1;

  if (my_time_a.second_part < my_time_b.second_part) return -1;
  if (my_time_a.second_part > my_time_b.second_part) return 1;

  return 0;
}


// Source: my_time.cc
// Lines 2788-2799
