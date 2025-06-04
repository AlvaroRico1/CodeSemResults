inline bool operator==(StringPiece x, StringPiece y) {
  StringPiece::size_type len = x.size();
  if (len != y.size()) {
    return false;
  }

  return x.data() == y.data() || len <= 0 ||
      memcmp(x.data(), y.data(), static_cast<size_t>(len)) == 0;
}

inline bool operator!=(StringPiece x, StringPiece y) {
  return !(x == y);
}

inline bool operator<(StringPiece x, StringPiece y) {
  const StringPiece::size_type min_size =
      x.size() < y.size() ? x.size() : y.size();
  const int r = memcmp(x.data(), y.data(), static_cast<size_t>(min_size));
  return (r < 0) || (r == 0 && x.size() < y.size());
}

inline bool operator>(StringPiece x, StringPiece y) {
  return y < x;
}

inline bool operator<=(StringPiece x, StringPiece y) {
  return !(x > y);
}

inline bool operator>=(StringPiece x, StringPiece y) {
  return !(x < y);
}

// allow StringPiece to be logged
extern std::ostream& operator<<(std::ostream& o, StringPiece piece);

}  // namespace stringpiece_internal


// Source: stringpiece.h
// Lines 345-381
