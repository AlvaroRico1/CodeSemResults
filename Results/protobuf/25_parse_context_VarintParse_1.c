PROTOBUF_NODISCARD const char* VarintParse(const char* p, T* out) {
  auto ptr = reinterpret_cast<const uint8_t*>(p);
  uint32_t res = ptr[0];
  if (!(res & 0x80)) {
    *out = res;
    return p + 1;
  }
  uint32_t byte = ptr[1];
  res += (byte - 1) << 7;
  if (!(byte & 0x80)) {
    *out = res;
    return p + 2;
  }
  return VarintParseSlow(p, res, out);
}


// Source: parse_context.h
// Lines 544-558
