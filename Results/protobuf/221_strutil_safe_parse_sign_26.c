inline bool safe_parse_sign(std::string *text /*inout*/,
                            bool *negative_ptr /*output*/) {
  const char* start = text->data();
  const char* end = start + text->size();

  // Consume whitespace.
  while (start < end && (start[0] == ' ')) {
    ++start;
  }
  while (start < end && (end[-1] == ' ')) {
    --end;
  }
  if (start >= end) {
    return false;
  }

  // Consume sign.
  *negative_ptr = (start[0] == '-');
  if (*negative_ptr || start[0] == '+') {
    ++start;
    if (start >= end) {
      return false;
    }
  }
  *text = text->substr(start - text->data(), end - start);
  return true;
}


// Source: strutil.cc
// Lines 666-692
