times_pten (REAL_VALUE_TYPE *r, int exp)
{
  REAL_VALUE_TYPE pten, *rr;
  bool negative = (exp < 0);
  int i;

  if (negative)
    {
      exp = -exp;
      pten = *real_digit (1);
      rr = &pten;
    }
  else
    rr = r;

  for (i = 0; exp > 0; ++i, exp >>= 1)
    if (exp & 1)
      do_multiply (rr, rr, ten_to_ptwo (i));

  if (negative)
    do_divide (r, r, &pten);
}


// Source: real.c
// Lines 2401-2422
