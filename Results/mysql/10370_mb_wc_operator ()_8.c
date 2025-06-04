  int operator()(my_wc_t *pwc, const uchar *s, const uchar *e) const {
    return my_mb_wc_utf8mb4(pwc, s, e);
  }
};


// Source: mb_wc.h
// Lines 84-87
