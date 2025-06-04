identifier_to_locale (const char *ident)
{
  const unsigned char *uid = (const unsigned char *) ident;
  size_t idlen = strlen (ident);
  bool valid_printable_utf8 = true;
  bool all_ascii = true;
  size_t i;

  for (i = 0; i < idlen;)
    {
      unsigned int c;
      size_t utf8_len = decode_utf8_char (&uid[i], idlen - i, &c);
      if (utf8_len == 0 || c <= 0x1F || (c >= 0x7F && c <= 0x9F))
	{
	  valid_printable_utf8 = false;
	  break;
	}
      if (utf8_len > 1)
	all_ascii = false;
      i += utf8_len;
    }


// Source: pretty-print.c
// Lines 2016-2036
