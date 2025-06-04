static Bigint *d2b(U *d, int *e, int *bits, Stack_alloc *alloc) {
  Bigint *b;
  int de, k;
  ULong *x, y, z;
  int i;
#define d0 word0(d)
#define d1 word1(d)

  b = Balloc(1, alloc);
  x = b->p.x;

  z = d0 & Frac_mask;
  d0 &= 0x7fffffff; /* clear sign bit, which we ignore */
  if ((de = (int)(d0 >> Exp_shift))) z |= Exp_msk1;
  if ((y = d1)) {
    if ((k = lo0bits(&y))) {
      x[0] = y | z << (32 - k);
      z >>= k;
    } else
      x[0] = y;
    i = b->wds = (x[1] = z) ? 2 : 1;
  } else {
    k = lo0bits(&z);
    x[0] = z;
    i = b->wds = 1;
    k += 32;
  }
  if (de) {
    *e = de - Bias - (P - 1) + k;
    *bits = P - k;
  } else {
    *e = de - Bias - (P - 1) + 1 + k;
    *bits = 32 * i - hi0bits(x[i - 1]);
  }
  return b;
#undef d0
#undef d1
}


// Source: dtoa.cc
// Lines 1170-1207
