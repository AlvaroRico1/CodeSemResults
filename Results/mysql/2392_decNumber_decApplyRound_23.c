static void decApplyRound(decNumber *dn, decContext *set, Int residue,
                          uInt *status) {
  Int  bump;                  /* 1 if coefficient needs to be incremented  */
                              /* -1 if coefficient needs to be decremented  */

  if (residue==0) return;     /* nothing to apply  */

  bump=0;                     /* assume a smooth ride  */

  /* now decide whether, and how, to round, depending on mode  */
  switch (set->round) {
    case DEC_ROUND_05UP: {    /* round zero or five up (for reround)  */
      /* This is the same as DEC_ROUND_DOWN unless there is a  */
      /* positive residue and the lsd of dn is 0 or 5, in which case  */
      /* it is bumped; when residue is <0, the number is therefore  */
      /* bumped down unless the final digit was 1 or 6 (in which  */
      /* case it is bumped down and then up -- a no-op)  */
      Int lsd5=*dn->lsu%5;     /* get lsd and quintate  */
      if (residue<0 && lsd5!=1) bump=-1;
       else if (residue>0 && lsd5==0) bump=1;
      /* [bump==1 could be applied directly; use common path for clarity]  */
      break;} /* r-05  */

    case DEC_ROUND_DOWN: {
      /* no change, except if negative residue  */
      if (residue<0) bump=-1;
      break;} /* r-d  */

    case DEC_ROUND_HALF_DOWN: {
      if (residue>5) bump=1;
      break;} /* r-h-d  */

    case DEC_ROUND_HALF_EVEN: {
      if (residue>5) bump=1;            /* >0.5 goes up  */
       else if (residue==5) {           /* exactly 0.5000...  */
        /* 0.5 goes up iff [new] lsd is odd  */
        if (*dn->lsu & 0x01) bump=1;
        }
      break;} /* r-h-e  */

    case DEC_ROUND_HALF_UP: {
      if (residue>=5) bump=1;
      break;} /* r-h-u  */

    case DEC_ROUND_UP: {
      if (residue>0) bump=1;
      break;} /* r-u  */

    case DEC_ROUND_CEILING: {
      /* same as _UP for positive numbers, and as _DOWN for negatives  */
      /* [negative residue cannot occur on 0]  */
      if (decNumberIsNegative(dn)) {
        if (residue<0) bump=-1;
        }
       else {
        if (residue>0) bump=1;
        }
      break;} /* r-c  */

    case DEC_ROUND_FLOOR: {
      /* same as _UP for negative numbers, and as _DOWN for positive  */
      /* [negative residue cannot occur on 0]  */
      if (!decNumberIsNegative(dn)) {
        if (residue<0) bump=-1;
        }
       else {
        if (residue>0) bump=1;
        }
      break;} /* r-f  */

    default: {      /* e.g., DEC_ROUND_MAX  */
      *status|=DEC_Invalid_context;
      #if DECTRACE || (DECCHECK && DECVERB)
      printf("Unknown rounding mode: %d\n", set->round);
      #endif
      break;}
    } /* switch  */

  /* now bump the number, up or down, if need be  */
  if (bump==0) return;                       /* no action required  */

  /* Simply use decUnitAddSub unless bumping up and the number is  */
  /* all nines.  In this special case set to 100... explicitly  */
  /* and adjust the exponent by one (as otherwise could overflow  */
  /* the array)  */
  /* Similarly handle all-nines result if bumping down.  */
  if (bump>0) {
    Unit *up;                                /* work  */
    uInt count=dn->digits;                   /* digits to be checked  */
    for (up=dn->lsu; ; up++) {
      if (count<=DECDPUN) {
        /* this is the last Unit (the msu)  */
        if (*up!=powers[count]-1) break;     /* not still 9s  */
        /* here if it, too, is all nines  */
        *up=(Unit)powers[count-1];           /* here 999 -> 100 etc.  */
        for (up=up-1; up>=dn->lsu; up--) *up=0; /* others all to 0  */
        dn->exponent++;                      /* and bump exponent  */
        /* [which, very rarely, could cause Overflow...]  */
        if ((dn->exponent+dn->digits)>set->emax+1) {
          decSetOverflow(dn, set, status);
          }
        return;                              /* done  */
        }
      /* a full unit to check, with more to come  */
      if (*up!=DECDPUNMAX) break;            /* not still 9s  */
      count-=DECDPUN;
      } /* up  */
    } /* bump>0  */


// Source: decNumber.cpp
// Lines 7117-7224
