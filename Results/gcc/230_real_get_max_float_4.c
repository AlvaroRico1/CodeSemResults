get_max_float (const struct real_format *fmt, char *buf, size_t len,
	       bool norm_max)
{
  int i, n;
  char *p;
  bool is_ibm_extended = fmt->pnan < fmt->p;

  strcpy (buf, "0x0.");
  n = fmt->p;
  for (i = 0, p = buf + 4; i + 3 < n; i += 4)
    *p++ = 'f';
  if (i < n)
    *p++ = "08ce"[n - i];
  sprintf (p, "p%d",
	   (is_ibm_extended && norm_max) ? fmt->emax - 1 : fmt->emax);
  if (is_ibm_extended && !norm_max)
    {
      /* This is an IBM extended double format made up of two IEEE
	 doubles.  The value of the long double is the sum of the
	 values of the two parts.  The most significant part is
	 required to be the value of the long double rounded to the
	 nearest double.  Rounding means we need a slightly smaller
	 value for LDBL_MAX.  */
      buf[4 + fmt->pnan / 4] = "7bde"[fmt->pnan % 4];
    }

  gcc_assert (strlen (buf) < len);
}


// Source: real.c
// Lines 5388-5415
