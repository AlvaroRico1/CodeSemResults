U_CAPI uByte * U_EXPORT2 uprv_decNumberGetBCD(const decNumber *dn, uByte *bcd) {
  uByte *ub=bcd+dn->digits-1;      /* -> lsd  */
  const Unit *up=dn->lsu;          /* Unit pointer, -> lsu  */

  #if DECDPUN==1                   /* trivial simple copy  */
    for (; ub>=bcd; ub--, up++) *ub=*up;
  #else                            /* chopping needed  */
    uInt u=*up;                    /* work  */
    uInt cut=DECDPUN;              /* downcounter through unit  */
    for (; ub>=bcd; ub--) {
      *ub=(uByte)(u%10);           /* [*6554 trick inhibits, here]  */
      u=u/10;
      cut--;
      if (cut>0) continue;         /* more in this unit  */
      up++;
      u=*up;
      cut=DECDPUN;
      }
  #endif
  return bcd;
  } /* decNumberGetBCD  */


// Source: decNumber.cpp
// Lines 3490-3510
