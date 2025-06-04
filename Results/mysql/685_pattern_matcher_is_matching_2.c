bool Pattern_matcher::is_matching(const std::string &text,
                                  const CHARSET_INFO *info) const {
  DBUG_TRACE;

  // traverse all patterns, return true on first match
  for (auto &pattern : m_patterns) {
    if (info->coll->wildcmp(info, text.c_str(), text.c_str() + text.length(),
                            pattern.c_str(), pattern.c_str() + pattern.length(),
                            WILD_ESCAPE, WILD_ONE, WILD_MANY) == 0) {
      return true;
    }
  }
  // none of the patterns matched
  return false;
}


// Source: pattern_matcher.cc
// Lines 76-90
