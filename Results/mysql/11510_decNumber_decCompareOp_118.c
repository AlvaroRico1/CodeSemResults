static decNumber * decCompareOp(decNumber *res, const decNumber *lhs,
                         const decNumber *rhs, decContext *set,
                         Flag op, uInt *status) {
  #if DECSUBSET
  decNumber *alloclhs=NULL;        /* non-NULL if rounded lhs allocated  */
  decNumber *allocrhs=NULL;        /* .., rhs  */
  #endif
  Int   result=0;                  /* default result value  */
  uByte merged;                    /* work  */

  #if DECCHECK
  if (decCheckOperands(res, lhs, rhs, set)) return res;
  #endif

  do {                             /* protect allocated storage  */
    #if DECSUBSET
    if (!set->extended) {
      /* reduce operands and set lostDigits status, as needed  */
      if (lhs->digits>set->digits) {
        alloclhs=decRoundOperand(lhs, set, status);
        if (alloclhs==NULL) {result=BADINT; break;}
        lhs=alloclhs;
        }
      if (rhs->digits>set->digits) {
        allocrhs=decRoundOperand(rhs, set, status);
        if (allocrhs==NULL) {result=BADINT; break;}
        rhs=allocrhs;
        }
      }
    #endif
    /* [following code does not require input rounding]  */

    /* If total ordering then handle differing signs 'up front'  */
    if (op==COMPTOTAL) {                /* total ordering  */
      if (decNumberIsNegative(lhs) && !decNumberIsNegative(rhs)) {
        result=-1;
        break;
        }
      if (!decNumberIsNegative(lhs) && decNumberIsNegative(rhs)) {
        result=+1;
        break;
        }
      }

    /* handle NaNs specially; let infinities drop through  */
    /* This assumes sNaN (even just one) leads to NaN.  */
    merged=(lhs->bits | rhs->bits) & (DECSNAN | DECNAN);
    if (merged) {                       /* a NaN bit set  */
      if (op==COMPARE);                 /* result will be NaN  */
       else if (op==COMPSIG)            /* treat qNaN as sNaN  */
        *status|=DEC_Invalid_operation | DEC_sNaN;
       else if (op==COMPTOTAL) {        /* total ordering, always finite  */
        /* signs are known to be the same; compute the ordering here  */
        /* as if the signs are both positive, then invert for negatives  */
        if (!decNumberIsNaN(lhs)) result=-1;
         else if (!decNumberIsNaN(rhs)) result=+1;
         /* here if both NaNs  */
         else if (decNumberIsSNaN(lhs) && decNumberIsQNaN(rhs)) result=-1;
         else if (decNumberIsQNaN(lhs) && decNumberIsSNaN(rhs)) result=+1;
         else { /* both NaN or both sNaN  */
          /* now it just depends on the payload  */
          result=decUnitCompare(lhs->lsu, D2U(lhs->digits),
                                rhs->lsu, D2U(rhs->digits), 0);
          /* [Error not possible, as these are 'aligned']  */
          } /* both same NaNs  */
        if (decNumberIsNegative(lhs)) result=-result;
        break;
        } /* total order  */

       else if (merged & DECSNAN);           /* sNaN -> qNaN  */
       else { /* here if MIN or MAX and one or two quiet NaNs  */
        /* min or max -- 754 rules ignore single NaN  */
        if (!decNumberIsNaN(lhs) || !decNumberIsNaN(rhs)) {
          /* just one NaN; force choice to be the non-NaN operand  */
          op=COMPMAX;
          if (lhs->bits & DECNAN) result=-1; /* pick rhs  */
                             else result=+1; /* pick lhs  */
          break;
          }
        } /* max or min  */
      op=COMPNAN;                            /* use special path  */
      decNaNs(res, lhs, rhs, set, status);   /* propagate NaN  */
      break;
      }
    /* have numbers  */
    if (op==COMPMAXMAG || op==COMPMINMAG) result=decCompare(lhs, rhs, 1);
     else result=decCompare(lhs, rhs, 0);    /* sign matters  */
    } while(0);                              /* end protected  */

  if (result==BADINT) *status|=DEC_Insufficient_storage; /* rare  */
   else {
    if (op==COMPARE || op==COMPSIG ||op==COMPTOTAL) { /* returning signum  */
      if (op==COMPTOTAL && result==0) {
        /* operands are numerically equal or same NaN (and same sign,  */
        /* tested first); if identical, leave result 0  */
        if (lhs->exponent!=rhs->exponent) {
          if (lhs->exponent<rhs->exponent) result=-1;
           else result=+1;
          if (decNumberIsNegative(lhs)) result=-result;
          } /* lexp!=rexp  */
        } /* total-order by exponent  */
      uprv_decNumberZero(res);               /* [always a valid result]  */
      if (result!=0) {                  /* must be -1 or +1  */
        *res->lsu=1;
        if (result<0) res->bits=DECNEG;
        }
      }
     else if (op==COMPNAN);             /* special, drop through  */
     else {                             /* MAX or MIN, non-NaN result  */
      Int residue=0;                    /* rounding accumulator  */
      /* choose the operand for the result  */
      const decNumber *choice;
      if (result==0) { /* operands are numerically equal  */
        /* choose according to sign then exponent (see 754)  */
        uByte slhs=(lhs->bits & DECNEG);
        uByte srhs=(rhs->bits & DECNEG);
        #if DECSUBSET
        if (!set->extended) {           /* subset: force left-hand  */
          op=COMPMAX;
          result=+1;
          }
        else
        #endif
        if (slhs!=srhs) {          /* signs differ  */
          if (slhs) result=-1;     /* rhs is max  */
               else result=+1;     /* lhs is max  */
          }
         else if (slhs && srhs) {  /* both negative  */
          if (lhs->exponent<rhs->exponent) result=+1;
                                      else result=-1;
          /* [if equal, use lhs, technically identical]  */
          }
         else {                    /* both positive  */
          if (lhs->exponent>rhs->exponent) result=+1;
                                      else result=-1;
          /* [ditto]  */
          }
        } /* numerically equal  */
      /* here result will be non-0; reverse if looking for MIN  */
      if (op==COMPMIN || op==COMPMINMAG) result=-result;
      choice=(result>0 ? lhs : rhs);    /* choose  */
      /* copy chosen to result, rounding if need be  */
      decCopyFit(res, choice, set, &residue, status);
      decFinish(res, set, &residue, status);
      }


// Source: decNumber.cpp
// Lines 6050-6194
