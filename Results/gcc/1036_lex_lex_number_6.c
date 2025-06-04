lex_number (cpp_reader *pfile, cpp_string *number,
	    struct normalize_state *nst)
{
  const uchar *cur;
  const uchar *base;
  uchar *dest;

  base = pfile->buffer->cur - 1;
  do
    {
      cur = pfile->buffer->cur;

      /* N.B. ISIDNUM does not include $.  */
      while (ISIDNUM (*cur) || *cur == '.' || DIGIT_SEP (*cur)
	     || VALID_SIGN (*cur, cur[-1]))
	{
	  NORMALIZE_STATE_UPDATE_IDNUM (nst, *cur);
	  cur++;
	}
      /* A number can't end with a digit separator.  */
      while (cur > pfile->buffer->cur && DIGIT_SEP (cur[-1]))
	--cur;

      pfile->buffer->cur = cur;
    }
  while (forms_identifier_p (pfile, false, nst));

  number->len = cur - base;
  dest = _cpp_unaligned_alloc (pfile, number->len + 1);
  memcpy (dest, base, number->len);
  dest[number->len] = '\0';
  number->text = dest;
}


// Source: lex.c
// Lines 1541-1573
