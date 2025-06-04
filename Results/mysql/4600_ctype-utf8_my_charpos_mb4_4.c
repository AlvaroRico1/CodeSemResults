size_t my_charpos_mb4(const CHARSET_INFO *cs, const char *pos, const char *end,
                      size_t length) {
  // Fast path as long as we see ASCII characters only.
  size_t min_length = std::min<size_t>(end - pos, length);
  const char *safe_end =
      std::min(end, pos + min_length) - std::min<size_t>(7, min_length);
  const char *start = pos;
  while (pos < safe_end) {
    uint64_t data;
    memcpy(&data, pos, sizeof(data));
    if (data & 0x8080808080808080ULL) break;
    pos += sizeof(data);
    length -= sizeof(data);
  }

  while (length && pos < end) {
    uint mb_len;
    pos += (mb_len = my_ismbchar_utf8mb4_inl(cs, pos, end)) ? mb_len : 1;
    length--;
  }
  return (size_t)(length ? end + 2 - start : pos - start);
}

static uint my_mbcharlen_utf8mb4(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                                 uint c) {
  if (c < 0x80) return 1;
  if (c < 0xc2) return 0; /* Illegal mb head */
  if (c < 0xe0) return 2;
  if (c < 0xf0) return 3;
  if (c < 0xf8) return 4;
  return 0; /* Illegal mb head */
  ;
}
}  // extern "C"


// Source: ctype-utf8.cc
// Lines 7706-7739
