U_CAPI decNumber * U_EXPORT2 uprv_decNumberAnd(decNumber *res, const decNumber *lhs,
                         const decNumber *rhs, decContext *set) {
  const Unit *ua, *ub;                  /* -> operands  */
  const Unit *msua, *msub;              /* -> operand msus  */
  Unit *uc,  *msuc;                     /* -> result and its msu  */
  Int   msudigs;                        /* digits in res msu  */
  #if DECCHECK
  if (decCheckOperands(res, lhs, rhs, set)) return res;
  #endif

  if (lhs->exponent!=0 || decNumberIsSpecial(lhs) || decNumberIsNegative(lhs)
   || rhs->exponent!=0 || decNumberIsSpecial(rhs) || decNumberIsNegative(rhs)) {
    decStatus(res, DEC_Invalid_operation, set);
    return res;
    }

  /* operands are valid  */
  ua=lhs->lsu;                          /* bottom-up  */
  ub=rhs->lsu;                          /* ..  */
  uc=res->lsu;                          /* ..  */
  msua=ua+D2U(lhs->digits)-1;           /* -> msu of lhs  */
  msub=ub+D2U(rhs->digits)-1;           /* -> msu of rhs  */
  msuc=uc+D2U(set->digits)-1;           /* -> msu of result  */
  msudigs=MSUDIGITS(set->digits);       /* [faster than remainder]  */
  for (; uc<=msuc; ua++, ub++, uc++) {  /* Unit loop  */
    Unit a, b;                          /* extract units  */
    if (ua>msua) a=0;
     else a=*ua;
    if (ub>msub) b=0;
     else b=*ub;
    *uc=0;                              /* can now write back  */
    if (a|b) {                          /* maybe 1 bits to examine  */
      Int i, j;
      *uc=0;                            /* can now write back  */
      /* This loop could be unrolled and/or use BIN2BCD tables  */
      for (i=0; i<DECDPUN; i++) {
        if (a&b&1) *uc=*uc+(Unit)powers[i];  /* effect AND  */
        j=a%10;
        a=a/10;
        j|=b%10;
        b=b/10;
        if (j>1) {
          decStatus(res, DEC_Invalid_operation, set);
          return res;
          }
        if (uc==msuc && i==msudigs-1) break; /* just did final digit  */
        } /* each digit  */
      } /* both OK  */
    } /* each unit  */
  /* [here uc-1 is the msu of the result]  */
  res->digits=decGetDigits(res->lsu, static_cast<int32_t>(uc - res->lsu));
  res->exponent=0;                      /* integer  */
  res->bits=0;                          /* sign=0  */
  return res;  /* [no status to set]  */
  } /* decNumberAnd  */


// Source: decNumber.cpp
// Lines 821-875
