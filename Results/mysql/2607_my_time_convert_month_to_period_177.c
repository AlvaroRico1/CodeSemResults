uint64_t convert_month_to_period(uint64_t month) {
  uint64_t year;
  if (month == 0L) return 0L;
  if ((year = month / 12) < 100) {
    year += (year < YY_PART_YEAR) ? 2000 : 1900;
  }
  return year * 100 + month % 12 + 1;
}


// Source: my_time.cc
// Lines 2272-2279
