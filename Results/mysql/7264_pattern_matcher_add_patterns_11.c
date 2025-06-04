size_t Pattern_matcher::add_patterns(const std::string &patterns,
                                     char delimiter) {
  DBUG_TRACE;
  size_t length = patterns.length();
  size_t pattern_count = 0;

  // we don't parse empty patterns
  if (length == 0) return pattern_count;

  size_t first = 0;
  size_t last = 0;
  do {
    // find end of the token
    if ((last = patterns.find(delimiter, first)) == std::string::npos)
      last = length;

    // we store only tokens that are not empty
    if (last - first > 0) {
      m_patterns.emplace(patterns, first, last - first);
      ++pattern_count;
    }

    first = last + 1;
  } while (last != length);

  return pattern_count;
}


// Source: pattern_matcher.cc
// Lines 38-64
