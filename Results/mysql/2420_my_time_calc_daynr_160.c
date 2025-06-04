long calc_daynr(uint year, uint month, uint day) {
  long delsum;
  int temp;
  int y = year; /* may be < 0 temporarily */

  if (y == 0 && month == 0) return 0; /* Skip errors */
  /* Cast to int to be able to handle month == 0 */
  delsum = static_cast<long>(365 * y + 31 * (static_cast<int>(month) - 1) +
                             static_cast<int>(day));
  if (month <= 2)
    y--;
  else
    delsum -= static_cast<long>(static_cast<int>(month) * 4 + 23) / 10;
  temp = ((y / 100 + 1) * 3) / 4;
  assert(delsum + static_cast<int>(y) / 4 - temp >= 0);
  return (delsum + static_cast<int>(y) / 4 - temp);
} /* calc_daynr */


// Source: my_time.cc
// Lines 1045-1061
