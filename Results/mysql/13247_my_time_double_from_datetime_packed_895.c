double double_from_datetime_packed(enum enum_field_types type,
                                   longlong packed_value) {
  longlong result = longlong_from_datetime_packed(type, packed_value);
  return result +
         (static_cast<double>(my_packed_time_get_frac_part(packed_value))) /
             1000000;
}


// Source: my_time.cc
// Lines 2891-2897
