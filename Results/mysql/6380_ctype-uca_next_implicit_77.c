ALWAYS_INLINE int uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::next_implicit(
    my_wc_t ch) {
  my_wc_t hangul_jamo[HANGUL_JAMO_MAX_LENGTH];
  int jamo_cnt;
  if ((jamo_cnt = my_decompose_hangul_syllable(ch, hangul_jamo))) {
    my_put_jamo_weights(hangul_jamo, jamo_cnt);
    num_of_ce_left = jamo_cnt - 1;
    wbeg = implicit + MY_UCA_900_CE_SIZE + weight_lv;
    wbeg_stride = MY_UCA_900_CE_SIZE;
    return *(implicit + weight_lv);
  }

  /*
    We give the Chinese collation different leading primary weight to make
    sure there are enough single weight values to be assigned to character
    groups like Latin, Cyrillic, etc.
  */
  uint page;
  if (ch >= 0x17000 && ch <= 0x18AFF)  // Tangut character
  {
    page = 0xFB00;
    implicit[3] = (ch - 0x17000) | 0x8000;
  } else {
    page = ch >> 15;
    implicit[3] = (ch & 0x7FFF) | 0x8000;
    if ((ch >= 0x3400 && ch <= 0x4DB5) || (ch >= 0x20000 && ch <= 0x2A6D6) ||
        (ch >= 0x2A700 && ch <= 0x2B734) || (ch >= 0x2B740 && ch <= 0x2B81D) ||
        (ch >= 0x2B820 && ch <= 0x2CEA1)) {
      page += 0xFB80;
    } else if ((ch >= 0x4E00 && ch <= 0x9FD5) || (ch >= 0xFA0E && ch <= 0xFA29))
      page += 0xFB40;
    else
      page += 0xFBC0;
  }
  if (cs->coll_param == &zh_coll_param) {
    page = change_zh_implicit(page);
  }
  implicit[0] = page;
  implicit[1] = 0x0020;
  implicit[2] = 0x0002;
  // implicit[3] is set above.
  implicit[4] = 0;
  implicit[5] = 0;
  num_of_ce_left = 1;
  wbeg = implicit + MY_UCA_900_CE_SIZE + weight_lv;
  wbeg_stride = MY_UCA_900_CE_SIZE;

  return *(implicit + weight_lv);
}


// Source: ctype-uca.cc
// Lines 1205-1253
