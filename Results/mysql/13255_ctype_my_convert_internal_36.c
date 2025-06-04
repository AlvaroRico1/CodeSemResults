static size_t my_convert_internal(char *to, size_t to_length,
                                  const CHARSET_INFO *to_cs, const char *from,
                                  size_t from_length,
                                  const CHARSET_INFO *from_cs, uint *errors) {
  int cnvres;
  my_wc_t wc;
  const uchar *from_end = (const uchar *)from + from_length;
  char *to_start = to;
  uchar *to_end = (uchar *)to + to_length;
  my_charset_conv_mb_wc mb_wc = from_cs->cset->mb_wc;
  my_charset_conv_wc_mb wc_mb = to_cs->cset->wc_mb;
  uint error_count = 0;

  while (true) {
    if ((cnvres = (*mb_wc)(from_cs, &wc, pointer_cast<const uchar *>(from),
                           from_end)) > 0)
      from += cnvres;
    else if (cnvres == MY_CS_ILSEQ) {
      error_count++;
      from++;
      wc = '?';
    } else if (cnvres > MY_CS_TOOSMALL) {
      /*
        A correct multibyte sequence detected
        But it doesn't have Unicode mapping.
      */
      error_count++;
      from += (-cnvres);
      wc = '?';
    } else
      break;  // Not enough characters

  outp:
    if ((cnvres = (*wc_mb)(to_cs, wc, (uchar *)to, to_end)) > 0)
      to += cnvres;
    else if (cnvres == MY_CS_ILUNI && wc != '?') {
      error_count++;
      wc = '?';
      goto outp;
    } else
      break;
  }
  *errors = error_count;
  return (uint32)(to - to_start);
}


// Source: ctype.cc
// Lines 872-916
