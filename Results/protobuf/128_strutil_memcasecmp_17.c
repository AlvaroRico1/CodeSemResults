static int memcasecmp(const char *s1, const char *s2, size_t len) {
  const unsigned char *us1 = reinterpret_cast<const unsigned char *>(s1);
  const unsigned char *us2 = reinterpret_cast<const unsigned char *>(s2);

  for (size_t i = 0; i < len; i++) {
    const int diff =
      static_cast<int>(static_cast<unsigned char>(ascii_tolower(us1[i]))) -
      static_cast<int>(static_cast<unsigned char>(ascii_tolower(us2[i])));
    if (diff != 0) return diff;
  }
  return 0;
}


// Source: strutil.cc
// Lines 1286-1297
