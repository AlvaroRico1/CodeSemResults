static int my_strnncoll_latin1_de(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                                  const uchar *a, size_t a_length,
                                  const uchar *b, size_t b_length,
                                  bool b_is_prefix) {
  const uchar *a_end = a + a_length;
  const uchar *b_end = b + b_length;
  uchar a_char, a_extend = 0, b_char, b_extend = 0;

  while ((a < a_end || a_extend) && (b < b_end || b_extend)) {
    if (a_extend) {
      a_char = a_extend;
      a_extend = 0;
    } else {
      a_extend = combo2map[*a];
      a_char = combo1map[*a++];
    }
    if (b_extend) {
      b_char = b_extend;
      b_extend = 0;
    } else {
      b_extend = combo2map[*b];
      b_char = combo1map[*b++];
    }
    if (a_char != b_char) return (int)a_char - (int)b_char;
  }
  /*
    A simple test of string lengths won't work -- we test to see
    which string ran out first
  */
  return ((a < a_end || a_extend) ? (b_is_prefix ? 0 : 1)
                                  : (b < b_end || b_extend) ? -1 : 0);
}

static int my_strnncollsp_latin1_de(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *a,
    size_t a_length, const uchar *b, size_t b_length) {
  const uchar *a_end = a + a_length, *b_end = b + b_length;
  uchar a_char, a_extend = 0, b_char, b_extend = 0;
  int res;

  while ((a < a_end || a_extend) && (b < b_end || b_extend)) {
    if (a_extend) {
      a_char = a_extend;
      a_extend = 0;
    } else {
      a_extend = combo2map[*a];
      a_char = combo1map[*a++];
    }
    if (b_extend) {
      b_char = b_extend;
      b_extend = 0;
    } else {
      b_extend = combo2map[*b];
      b_char = combo1map[*b++];
    }
    if (a_char != b_char) return (int)a_char - (int)b_char;
  }
  /* Check if double character last */
  if (a_extend) return 1;
  if (b_extend) return -1;

  res = 0;
  if (a != a_end || b != b_end) {
    int swap = 1;
    /*
      Check the next not space character of the longer key. If it's < ' ',
      then it's smaller than the other key.
    */
    if (a == a_end) {
      /* put shorter key in a */
      a_end = b_end;
      a = b;
      swap = -1; /* swap sign of result */
      res = -res;
    }
    for (; a < a_end; a++) {
      if (*a != ' ') return (*a < ' ') ? -swap : swap;
    }
  }
  return res;
}

static size_t my_strnxfrm_latin1_de(const CHARSET_INFO *cs, uchar *dst,
                                    size_t dstlen, uint nweights,
                                    const uchar *src, size_t srclen,
                                    uint flags) {
  uchar *de = dst + dstlen;
  const uchar *se = src + srclen;
  uchar *d0 = dst;
  for (; src < se && dst < de && nweights; src++, nweights--) {
    uchar chr = combo1map[*src];
    *dst++ = chr;
    if ((chr = combo2map[*src]) && dst < de) {
      *dst++ = chr;
    }
  }
  return my_strxfrm_pad(cs, d0, dst, de, nweights, flags);
}

static void my_hash_sort_latin1_de(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), const uchar *key, size_t len,
    uint64 *nr1, uint64 *nr2) {
  const uchar *end;
  uint64 tmp1;
  uint64 tmp2;

  /*
    Remove end space. We have to do this to be able to compare
    'AE' and 'Ä' as identical
  */
  end = skip_trailing_space(key, len);

  tmp1 = *nr1;
  tmp2 = *nr2;

  for (; key < end; key++) {
    uint X = (uint)combo1map[(uint)*key];
    tmp1 ^= (uint64)((((uint)tmp1 & 63) + tmp2) * X) + (tmp1 << 8);
    tmp2 += 3;
    if ((X = combo2map[*key])) {
      tmp1 ^= (uint64)((((uint)tmp1 & 63) + tmp2) * X) + (tmp1 << 8);
      tmp2 += 3;
    }
  }

  *nr1 = tmp1;
  *nr2 = tmp2;
}
}  // extern "C"


// Source: ctype-latin1.cc
// Lines 491-619
