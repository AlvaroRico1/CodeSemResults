uint my_string_repertoire(const CHARSET_INFO *cs, const char *str,
                          size_t length) {
  const char *strend = str + length;
  if (cs->mbminlen == 1) {
    for (; str < strend; str++) {
      if (((uchar)*str) > 0x7F) return MY_REPERTOIRE_UNICODE30;
    }
  } else {
    my_wc_t wc;
    int chlen;
    for (; (chlen = cs->cset->mb_wc(cs, &wc, pointer_cast<const uchar *>(str),
                                    pointer_cast<const uchar *>(strend))) > 0;
         str += chlen) {
      if (wc > 0x7F) return MY_REPERTOIRE_UNICODE30;
    }
  }
  return MY_REPERTOIRE_ASCII;
}


// Source: ctype.cc
// Lines 768-785
