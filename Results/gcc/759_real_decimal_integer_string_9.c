decimal_integer_string (char *str, const REAL_VALUE_TYPE *r_orig,
			size_t buf_size)
{
  int dec_exp, digit, digits;
  REAL_VALUE_TYPE r, pten;
  char *p;
  bool sign;

  r = *r_orig;

  if (r.cl == rvc_zero)
    {
      strcpy (str, "0.");
      return;
    }

  sign = r.sign;
  r.sign = 0;

  dec_exp = REAL_EXP (&r) * M_LOG10_2;
  digits = dec_exp + 1;
  gcc_assert ((digits + 2) < (int)buf_size);

  pten = *real_digit (1);
  times_pten (&pten, dec_exp);

  p = str;
  if (sign)
    *p++ = '-';

  digit = rtd_divmod (&r, &pten);
  gcc_assert (digit >= 0 && digit <= 9);
  *p++ = digit + '0';
  while (--digits > 0)
    {
      times_pten (&r, 1);
      digit = rtd_divmod (&r, &pten);
      *p++ = digit + '0';
    }
  *p++ = '.';
  *p++ = '\0';
}


// Source: real.c
// Lines 2280-2321
