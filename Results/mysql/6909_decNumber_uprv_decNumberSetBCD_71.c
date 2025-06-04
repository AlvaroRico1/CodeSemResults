U_CAPI decNumber * U_EXPORT2 uprv_decNumberSetBCD(decNumber *dn, const uByte *bcd, uInt n) {
  Unit *up=dn->lsu+D2U(dn->digits)-1;   /* -> msu [target pointer]  */
  const uByte *ub=bcd;                  /* -> source msd  */

  #if DECDPUN==1                        /* trivial simple copy  */
    for (; ub<bcd+n; ub++, up--) *up=*ub;
  #else                                 /* some assembly needed  */
    /* calculate how many digits in msu, and hence first cut  */
    Int cut=MSUDIGITS(n);               /* [faster than remainder]  */
    for (;up>=dn->lsu; up--) {          /* each Unit from msu  */
      *up=0;                            /* will take <=DECDPUN digits  */
      for (; cut>0; ub++, cut--) *up=X10(*up)+*ub;
      cut=DECDPUN;                      /* next Unit has all digits  */
      }
  #endif
  dn->digits=n;                         /* set digit count  */
  return dn;
  } /* decNumberSetBCD  */


// Source: decNumber.cpp
// Lines 3524-3541
