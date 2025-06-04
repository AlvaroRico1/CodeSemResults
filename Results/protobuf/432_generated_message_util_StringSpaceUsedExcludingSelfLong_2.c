size_t StringSpaceUsedExcludingSelfLong(const std::string& str) {
  const void* start = &str;
  const void* end = &str + 1;
  if (start <= str.data() && str.data() < end) {
    // The string's data is stored inside the string object itself.
    return 0;
  } else {
    return str.capacity();
  }
}


// Source: generated_message_util.cc
// Lines 92-101
