bool safe_uint_internal(std::string text, IntType *value_p) {
  *value_p = 0;
  bool negative;
  if (!safe_parse_sign(&text, &negative) || negative) {
    return false;
  }
  return safe_parse_positive_int(text, value_p);
}


// Source: strutil.cc
// Lines 782-789
