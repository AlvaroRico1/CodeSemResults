static size_t my_strnxfrm_big5(const CHARSET_INFO *cs, uchar *dst,
                               size_t dstlen, uint nweights, const uchar *src,
                               size_t srclen, uint flags) {
  uchar *d0 = dst;
  uchar *de = dst + dstlen;
  const uchar *se = src + srclen;
  const uchar *sort_order = cs->sort_order;

  for (; dst < de && src < se && nweights; nweights--) {
    if (cs->cset->ismbchar(cs, (const char *)src, (const char *)se)) {
      /*
        Note, it is safe not to check (src < se)
        in the code below, because ismbchar() would
        not return TRUE if src was too short
      */
      uint16 e = big5strokexfrm((uint16)big5code(*src, *(src + 1)));
      *dst++ = big5head(e);
      if (dst < de) *dst++ = big5tail(e);
      src += 2;
    } else
      *dst++ = sort_order ? sort_order[*src++] : *src++;
  }
  return my_strxfrm_pad(cs, d0, dst, de, nweights, flags);
}


// Source: ctype-big5.cc
// Lines 1260-1283
