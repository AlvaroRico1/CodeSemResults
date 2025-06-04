bool safe_int_internal(std::string text, IntType *value_p) {
  *value_p = 0;
  bool negative;
  if (!safe_parse_sign(&text, &negative)) {
    return false;
  }
  if (!negative) {
    return safe_parse_positive_int(text, value_p);
  } else {
    return safe_parse_negative_int(text, value_p);
  }
}


// Source: strutil.cc
// Lines 768-779
