static char *write_length_encoded_string4(char *dest, char *dest_end,
                                          const uchar *src,
                                          const uchar *src_end) {
  size_t src_len = (size_t)(src_end - src);
  uchar *to = net_store_length((uchar *)dest, src_len);
  if ((char *)(to + src_len) >= dest_end) return nullptr;
  memcpy(to, src, src_len);
  return (char *)(to + src_len);
}


// Source: client.cc
// Lines 3930-3938
