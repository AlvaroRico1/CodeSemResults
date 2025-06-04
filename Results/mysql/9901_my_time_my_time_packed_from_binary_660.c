longlong my_time_packed_from_binary(const uchar *ptr, uint dec) {
  assert(dec <= DATETIME_MAX_DECIMALS);

  switch (dec) {
    case 0:
    default: {
      longlong intpart = mi_uint3korr(ptr) - TIMEF_INT_OFS;
      return my_packed_time_make_int(intpart);
    }


// Source: my_time.cc
// Lines 1784-1792
