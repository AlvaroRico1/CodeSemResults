do_multiply (REAL_VALUE_TYPE *r, const REAL_VALUE_TYPE *a,
	     const REAL_VALUE_TYPE *b)
{
  REAL_VALUE_TYPE u, t, *rr;
  unsigned int i, j, k;
  int sign = a->sign ^ b->sign;
  bool inexact = false;

  switch (CLASS2 (a->cl, b->cl))
    {
    case CLASS2 (rvc_zero, rvc_zero):
    case CLASS2 (rvc_zero, rvc_normal):
    case CLASS2 (rvc_normal, rvc_zero):
      /* +-0 * ANY = 0 with appropriate sign.  */
      get_zero (r, sign);
      return false;

    case CLASS2 (rvc_zero, rvc_nan):
    case CLASS2 (rvc_normal, rvc_nan):
    case CLASS2 (rvc_inf, rvc_nan):
    case CLASS2 (rvc_nan, rvc_nan):
      /* ANY * NaN = NaN.  */
      *r = *b;
      /* Make resulting NaN value to be qNaN. The caller has the
         responsibility to avoid the operation if flag_signaling_nans
         is on.  */
      r->signalling = 0;
      r->sign = sign;
      return false;

    case CLASS2 (rvc_nan, rvc_zero):
    case CLASS2 (rvc_nan, rvc_normal):
    case CLASS2 (rvc_nan, rvc_inf):
      /* NaN * ANY = NaN.  */
      *r = *a;
      /* Make resulting NaN value to be qNaN. The caller has the
         responsibility to avoid the operation if flag_signaling_nans
         is on.  */
      r->signalling = 0;
      r->sign = sign;
      return false;

    case CLASS2 (rvc_zero, rvc_inf):
    case CLASS2 (rvc_inf, rvc_zero):
      /* 0 * Inf = NaN */
      get_canonical_qnan (r, sign);
      return false;

    case CLASS2 (rvc_inf, rvc_inf):
    case CLASS2 (rvc_normal, rvc_inf):
    case CLASS2 (rvc_inf, rvc_normal):
      /* Inf * Inf = Inf, R * Inf = Inf */
      get_inf (r, sign);
      return false;

    case CLASS2 (rvc_normal, rvc_normal):
      break;

    default:
      gcc_unreachable ();
    }

  if (r == a || r == b)
    rr = &t;
  else
    rr = r;
  get_zero (rr, 0);

  /* Collect all the partial products.  Since we don't have sure access
     to a widening multiply, we split each long into two half-words.

     Consider the long-hand form of a four half-word multiplication:

		 A  B  C  D
	      *  E  F  G  H
	     --------------
	        DE DF DG DH
	     CE CF CG CH
	  BE BF BG BH
       AE AF AG AH

     We construct partial products of the widened half-word products
     that are known to not overlap, e.g. DF+DH.  Each such partial
     product is given its proper exponent, which allows us to sum them
     and obtain the finished product.  */

  for (i = 0; i < SIGSZ * 2; ++i)
    {
      unsigned long ai = a->sig[i / 2];
      if (i & 1)
	ai >>= HOST_BITS_PER_LONG / 2;
      else
	ai &= ((unsigned long)1 << (HOST_BITS_PER_LONG / 2)) - 1;

      if (ai == 0)
	continue;

      for (j = 0; j < 2; ++j)
	{
	  int exp = (REAL_EXP (a) - (2*SIGSZ-1-i)*(HOST_BITS_PER_LONG/2)
		     + (REAL_EXP (b) - (1-j)*(HOST_BITS_PER_LONG/2)));

	  if (exp > MAX_EXP)
	    {
	      get_inf (r, sign);
	      return true;
	    }
	  if (exp < -MAX_EXP)
	    {
	      /* Would underflow to zero, which we shouldn't bother adding.  */
	      inexact = true;
	      continue;
	    }

	  memset (&u, 0, sizeof (u));
	  u.cl = rvc_normal;
	  SET_REAL_EXP (&u, exp);

	  for (k = j; k < SIGSZ * 2; k += 2)
	    {
	      unsigned long bi = b->sig[k / 2];
	      if (k & 1)
		bi >>= HOST_BITS_PER_LONG / 2;
	      else
		bi &= ((unsigned long)1 << (HOST_BITS_PER_LONG / 2)) - 1;

	      u.sig[k / 2] = ai * bi;
	    }

	  normalize (&u);
	  inexact |= do_add (rr, rr, &u, 0);
	}
    }

  rr->sign = sign;
  if (rr != r)
    *r = t;

  return inexact;
}


// Source: real.c
// Lines 667-806
