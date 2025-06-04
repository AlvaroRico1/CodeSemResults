size_t well_formed_copy_nchars(const CHARSET_INFO *to_cs, char *to,
                               size_t to_length, const CHARSET_INFO *from_cs,
                               const char *from, size_t from_length,
                               size_t nchars,
                               const char **well_formed_error_pos,
                               const char **cannot_convert_error_pos,
                               const char **from_end_pos) {
  size_t res;

  if ((to_cs == &my_charset_bin) || (from_cs == &my_charset_bin) ||
      (to_cs == from_cs) || my_charset_same(from_cs, to_cs)) {
    if (to_length < to_cs->mbminlen || !nchars) {
      *from_end_pos = from;
      *cannot_convert_error_pos = nullptr;
      *well_formed_error_pos = nullptr;
      return 0;
    }

    if (to_cs == &my_charset_bin) {
      res = min(min(nchars, to_length), from_length);
      memmove(to, from, res);
      *from_end_pos = from + res;
      *well_formed_error_pos = nullptr;
      *cannot_convert_error_pos = nullptr;
    } else {
      int well_formed_error;
      uint from_offset;

      if ((from_offset = (from_length % to_cs->mbminlen)) &&
          (from_cs == &my_charset_bin)) {
        /*
          Copying from BINARY to UCS2 needs to prepend zeros sometimes:
          INSERT INTO t1 (ucs2_column) VALUES (0x01);
          0x01 -> 0x0001
        */
        uint pad_length = to_cs->mbminlen - from_offset;
        memset(to, 0, pad_length);
        memmove(to + pad_length, from, from_offset);
        /*
          In some cases left zero-padding can create an incorrect character.
          For example:
            INSERT INTO t1 (utf32_column) VALUES (0x110000);
          We'll pad the value to 0x00110000, which is a wrong UTF32 sequence!
          The valid characters range is limited to 0x00000000..0x0010FFFF.

          Make sure we didn't pad to an incorrect character.
        */
        if (to_cs->cset->well_formed_len(to_cs, to, to + to_cs->mbminlen, 1,
                                         &well_formed_error) !=
            to_cs->mbminlen) {
          *from_end_pos = *well_formed_error_pos = from;
          *cannot_convert_error_pos = nullptr;
          return 0;
        }
        nchars--;
        from += from_offset;
        from_length -= from_offset;
        to += to_cs->mbminlen;
        to_length -= to_cs->mbminlen;
      }

      size_t min_length = min(from_length, to_length);
      /*
        If we operate on a multi-byte fixed-width character set, make
        sure the string wasn't truncated in the middle of a character.
        If so, truncate to a character boundary.
      */
      if (to_cs->mbmaxlen > 1 && to_cs->mbmaxlen == to_cs->mbminlen)
        min_length -= min_length % to_cs->mbmaxlen;

      res = to_cs->cset->well_formed_len(to_cs, from, from + min_length, nchars,
                                         &well_formed_error);
      if (res > 0) memmove(to, from, res);
      *from_end_pos = from + res;

      /*
        If we are operating on a multi-byte variable-width character set and
        "well_formed_error" is set to true, it means the string is not
        well-formed (either partially or completely). But, in case of a
        well-formed string whose string length is too long for the destination
        buffer (i.e to_length), there is a possibility that the string is
        truncated in the middle of a character, which would break its sequence.
        This could lead to mistakenly rejecting the string as malformed.

        To resolve this, we check if the full string contains a valid character
        immediately after the returned end point. If it does, the string was
        just truncated; if it doesn't, it was actually malformed.

        For example: Consider a well-formed string of 300 bytes, which contained
        a given three-byte code point at str[254..256]. Furthermore, suppose the
        destination buffer is 255 bytes long. In this case, well_formed_len()
        would return 254, so we would need to check the bytes str[254..257] to
        see if they contain a well-formed character. This second invocation of
        well_formed_len() would return 2, which takes us across the truncation
        point, confirming that the problem was indeed truncation and not a
        malformed code point.
      */
      if (well_formed_error && to_cs->mbmaxlen > 1 &&
          res > min_length - to_cs->mbmaxlen) {
        const char *from_end = from + min(res + to_cs->mbmaxlen, from_length);

        size_t extra = to_cs->cset->well_formed_len(
            to_cs, *from_end_pos, from_end, 1, &well_formed_error);

        well_formed_error = (res + extra < min_length);
      }

      *well_formed_error_pos = well_formed_error ? *from_end_pos : nullptr;
      *cannot_convert_error_pos = nullptr;
      if (from_offset) res += to_cs->mbminlen;
    }
  } else {
    int cnvres;
    my_wc_t wc;
    my_charset_conv_mb_wc mb_wc = from_cs->cset->mb_wc;
    my_charset_conv_wc_mb wc_mb = to_cs->cset->wc_mb;
    const uchar *from_end = (const uchar *)from + from_length;
    uchar *to_end = (uchar *)to + to_length;
    char *to_start = to;
    *well_formed_error_pos = nullptr;
    *cannot_convert_error_pos = nullptr;

    for (; nchars; nchars--) {
      const char *from_prev = from;
      if ((cnvres = (*mb_wc)(from_cs, &wc, pointer_cast<const uchar *>(from),
                             from_end)) > 0)
        from += cnvres;
      else if (cnvres == MY_CS_ILSEQ) {
        if (!*well_formed_error_pos) *well_formed_error_pos = from;
        from++;
        wc = '?';
      } else if (cnvres > MY_CS_TOOSMALL) {
        /*
          A correct multibyte sequence detected
          But it doesn't have Unicode mapping.
        */
        if (!*cannot_convert_error_pos) *cannot_convert_error_pos = from;
        from += (-cnvres);
        wc = '?';
      } else
        break;  // Not enough characters

    outp:
      if ((cnvres = (*wc_mb)(to_cs, wc, (uchar *)to, to_end)) > 0)
        to += cnvres;
      else if (cnvres == MY_CS_ILUNI && wc != '?') {
        if (!*cannot_convert_error_pos) *cannot_convert_error_pos = from_prev;
        wc = '?';
        goto outp;
      } else {
        from = from_prev;
        break;
      }
    }
    *from_end_pos = from;
    res = to - to_start;
  }


// Source: sql_string.cc
// Lines 821-977
