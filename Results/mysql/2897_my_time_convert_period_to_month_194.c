uint64_t convert_period_to_month(uint64_t period) {
  uint64_t a;
  unsigned b;
  if (period == 0) return 0L;
  if ((a = period / 100) < YY_PART_YEAR)
    a += 2000;
  else if (a < 100)
    a += 1900;
  b = period % 100;
  return a * 12 + b - 1;
}


// Source: my_time.cc
// Lines 2255-2265
