static int my_strcasecmp_uca(const CHARSET_INFO *cs, const char *s,
                             const char *t) {
  const MY_UNICASE_INFO *uni_plane = cs->caseinfo;
  const MY_UNICASE_CHARACTER *page;
  while (s[0] && t[0]) {
    my_wc_t s_wc, t_wc;

    if (static_cast<uchar>(s[0]) < 128) {
      s_wc = uni_plane->page[0][static_cast<uchar>(s[0])].tolower;
      s++;
    } else {
      int res;

      res = cs->cset->mb_wc(cs, &s_wc, pointer_cast<const uchar *>(s),
                            pointer_cast<const uchar *>(s + 4));

      if (res <= 0) return strcmp(s, t);
      s += res;
      if (s_wc <= uni_plane->maxchar && (page = uni_plane->page[s_wc >> 8]))
        s_wc = page[s_wc & 0xFF].tolower;
    }

    /* Do the same for the second string */

    if (static_cast<uchar>(t[0]) < 128) {
      /* Convert single byte character into weight */
      t_wc = uni_plane->page[0][static_cast<uchar>(t[0])].tolower;
      t++;
    } else {
      int res = cs->cset->mb_wc(cs, &t_wc, pointer_cast<const uchar *>(t),
                                pointer_cast<const uchar *>(t + 4));
      if (res <= 0) return strcmp(s, t);
      t += res;

      if (t_wc <= uni_plane->maxchar && (page = uni_plane->page[t_wc >> 8]))
        t_wc = page[t_wc & 0xFF].tolower;
    }

    /* Now we have two weights, let's compare them */
    if (s_wc != t_wc) return static_cast<int>(s_wc) - static_cast<int>(t_wc);
  }


// Source: ctype-uca.cc
// Lines 2354-2394
