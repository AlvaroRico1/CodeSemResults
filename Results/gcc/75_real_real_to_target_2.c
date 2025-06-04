real_to_target (long *buf, const REAL_VALUE_TYPE *r_orig,
		format_helper fmt)
{
  REAL_VALUE_TYPE r;
  long buf1;

  r = *r_orig;
  round_for_format (fmt, &r);

  if (!buf)
    buf = &buf1;
  (*fmt->encode) (fmt, buf, &r);

  return *buf;
}


// Source: real.c
// Lines 2827-2841
