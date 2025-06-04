static char *write_string(char *dest, char *dest_end, const uchar *src,
                          const uchar *src_end) {
  size_t src_len = (size_t)(src_end - src);
  uchar *to = nullptr;
  if (src_len >= 251) return nullptr;
  *dest = (uchar)src_len;
  to = (uchar *)dest + 1;
  if ((char *)(to + src_len) >= dest_end) return nullptr;
  memcpy(to, src, src_len);
  return (char *)(to + src_len);
}


// Source: client.cc
// Lines 3944-3954
