static decNumber * decDivideOp(decNumber *res,
                               const decNumber *lhs, const decNumber *rhs,
                               decContext *set, Flag op, uInt *status) {
  #if DECSUBSET
  decNumber *alloclhs=NULL;        /* non-NULL if rounded lhs allocated  */
  decNumber *allocrhs=NULL;        /* .., rhs  */
  #endif
  Unit  accbuff[SD2U(DECBUFFER+DECDPUN+10)]; /* local buffer  */
  Unit  *acc=accbuff;              /* -> accumulator array for result  */
  Unit  *allocacc=NULL;            /* -> allocated buffer, iff allocated  */
  Unit  *accnext;                  /* -> where next digit will go  */
  Int   acclength;                 /* length of acc needed [Units]  */
  Int   accunits;                  /* count of units accumulated  */
  Int   accdigits;                 /* count of digits accumulated  */

  Unit  varbuff[SD2U(DECBUFFER*2+DECDPUN)];  /* buffer for var1  */
  Unit  *var1=varbuff;             /* -> var1 array for long subtraction  */
  Unit  *varalloc=NULL;            /* -> allocated buffer, iff used  */
  Unit  *msu1;                     /* -> msu of var1  */

  const Unit *var2;                /* -> var2 array  */
  const Unit *msu2;                /* -> msu of var2  */
  Int   msu2plus;                  /* msu2 plus one [does not vary]  */
  eInt  msu2pair;                  /* msu2 pair plus one [does not vary]  */

  Int   var1units, var2units;      /* actual lengths  */
  Int   var2ulen;                  /* logical length (units)  */
  Int   var1initpad=0;             /* var1 initial padding (digits)  */
  Int   maxdigits;                 /* longest LHS or required acc length  */
  Int   mult;                      /* multiplier for subtraction  */
  Unit  thisunit;                  /* current unit being accumulated  */
  Int   residue;                   /* for rounding  */
  Int   reqdigits=set->digits;     /* requested DIGITS  */
  Int   exponent;                  /* working exponent  */
  Int   maxexponent=0;             /* DIVIDE maximum exponent if unrounded  */
  uByte bits;                      /* working sign  */
  Unit  *target;                   /* work  */
  const Unit *source;              /* ..  */
  uInt  const *pow;                /* ..  */
  Int   shift, cut;                /* ..  */
  #if DECSUBSET
  Int   dropped;                   /* work  */
  #endif

  #if DECCHECK
  if (decCheckOperands(res, lhs, rhs, set)) return res;
  #endif

  do {                             /* protect allocated storage  */
    #if DECSUBSET
    if (!set->extended) {
      /* reduce operands and set lostDigits status, as needed  */
      if (lhs->digits>reqdigits) {
        alloclhs=decRoundOperand(lhs, set, status);
        if (alloclhs==NULL) break;
        lhs=alloclhs;
        }
      if (rhs->digits>reqdigits) {
        allocrhs=decRoundOperand(rhs, set, status);
        if (allocrhs==NULL) break;
        rhs=allocrhs;
        }
      }
    #endif
    /* [following code does not require input rounding]  */

    bits=(lhs->bits^rhs->bits)&DECNEG;  /* assumed sign for divisions  */

    /* handle infinities and NaNs  */
    if (SPECIALARGS) {                  /* a special bit set  */
      if (SPECIALARGS & (DECSNAN | DECNAN)) { /* one or two NaNs  */
        decNaNs(res, lhs, rhs, set, status);
        break;
        }
      /* one or two infinities  */
      if (decNumberIsInfinite(lhs)) {   /* LHS (dividend) is infinite  */
        if (decNumberIsInfinite(rhs) || /* two infinities are invalid ..  */
            op & (REMAINDER | REMNEAR)) { /* as is remainder of infinity  */
          *status|=DEC_Invalid_operation;
          break;
          }
        /* [Note that infinity/0 raises no exceptions]  */
        uprv_decNumberZero(res);
        res->bits=bits|DECINF;          /* set +/- infinity  */
        break;
        }
       else {                           /* RHS (divisor) is infinite  */
        residue=0;
        if (op&(REMAINDER|REMNEAR)) {
          /* result is [finished clone of] lhs  */
          decCopyFit(res, lhs, set, &residue, status);
          }
         else {  /* a division  */
          uprv_decNumberZero(res);
          res->bits=bits;               /* set +/- zero  */
          /* for DIVIDEINT the exponent is always 0.  For DIVIDE, result  */
          /* is a 0 with infinitely negative exponent, clamped to minimum  */
          if (op&DIVIDE) {
            res->exponent=set->emin-set->digits+1;
            *status|=DEC_Clamped;
            }
          }
        decFinish(res, set, &residue, status);
        break;
        }
      }

    /* handle 0 rhs (x/0)  */
    if (ISZERO(rhs)) {                  /* x/0 is always exceptional  */
      if (ISZERO(lhs)) {
        uprv_decNumberZero(res);             /* [after lhs test]  */
        *status|=DEC_Division_undefined;/* 0/0 will become NaN  */
        }
       else {
        uprv_decNumberZero(res);
        if (op&(REMAINDER|REMNEAR)) *status|=DEC_Invalid_operation;
         else {
          *status|=DEC_Division_by_zero; /* x/0  */
          res->bits=bits|DECINF;         /* .. is +/- Infinity  */
          }
        }
      break;}

    /* handle 0 lhs (0/x)  */
    if (ISZERO(lhs)) {                  /* 0/x [x!=0]  */
      #if DECSUBSET
      if (!set->extended) uprv_decNumberZero(res);
       else {
      #endif
        if (op&DIVIDE) {
          residue=0;
          exponent=lhs->exponent-rhs->exponent; /* ideal exponent  */
          uprv_decNumberCopy(res, lhs);      /* [zeros always fit]  */
          res->bits=bits;               /* sign as computed  */
          res->exponent=exponent;       /* exponent, too  */
          decFinalize(res, set, &residue, status);   /* check exponent  */
          }
         else if (op&DIVIDEINT) {
          uprv_decNumberZero(res);           /* integer 0  */
          res->bits=bits;               /* sign as computed  */
          }
         else {                         /* a remainder  */
          exponent=rhs->exponent;       /* [save in case overwrite]  */
          uprv_decNumberCopy(res, lhs);      /* [zeros always fit]  */
          if (exponent<res->exponent) res->exponent=exponent; /* use lower  */
          }
      #if DECSUBSET
        }
      #endif
      break;}

    /* Precalculate exponent.  This starts off adjusted (and hence fits  */
    /* in 31 bits) and becomes the usual unadjusted exponent as the  */
    /* division proceeds.  The order of evaluation is important, here,  */
    /* to avoid wrap.  */
    exponent=(lhs->exponent+lhs->digits)-(rhs->exponent+rhs->digits);

    /* If the working exponent is -ve, then some quick exits are  */
    /* possible because the quotient is known to be <1  */
    /* [for REMNEAR, it needs to be < -1, as -0.5 could need work]  */
    if (exponent<0 && !(op==DIVIDE)) {
      if (op&DIVIDEINT) {
        uprv_decNumberZero(res);                  /* integer part is 0  */
        #if DECSUBSET
        if (set->extended)
        #endif
          res->bits=bits;                    /* set +/- zero  */
        break;}
      /* fastpath remainders so long as the lhs has the smaller  */
      /* (or equal) exponent  */
      if (lhs->exponent<=rhs->exponent) {
        if (op&REMAINDER || exponent<-1) {
          /* It is REMAINDER or safe REMNEAR; result is [finished  */
          /* clone of] lhs  (r = x - 0*y)  */
          residue=0;
          decCopyFit(res, lhs, set, &residue, status);
          decFinish(res, set, &residue, status);
          break;
          }
        /* [unsafe REMNEAR drops through]  */
        }
      } /* fastpaths  */

    /* Long (slow) division is needed; roll up the sleeves... */

    /* The accumulator will hold the quotient of the division.  */
    /* If it needs to be too long for stack storage, then allocate.  */
    acclength=D2U(reqdigits+DECDPUN);   /* in Units  */
    if (acclength*sizeof(Unit)>sizeof(accbuff)) {
      /* printf("malloc dvacc %ld units\n", acclength);  */
      allocacc=(Unit *)malloc(acclength*sizeof(Unit));
      if (allocacc==NULL) {             /* hopeless -- abandon  */
        *status|=DEC_Insufficient_storage;
        break;}
      acc=allocacc;                     /* use the allocated space  */
      }

    /* var1 is the padded LHS ready for subtractions.  */
    /* If it needs to be too long for stack storage, then allocate.  */
    /* The maximum units needed for var1 (long subtraction) is:  */
    /* Enough for  */
    /*     (rhs->digits+reqdigits-1) -- to allow full slide to right  */
    /* or  (lhs->digits)             -- to allow for long lhs  */
    /* whichever is larger  */
    /*   +1                -- for rounding of slide to right  */
    /*   +1                -- for leading 0s  */
    /*   +1                -- for pre-adjust if a remainder or DIVIDEINT  */
    /* [Note: unused units do not participate in decUnitAddSub data]  */
    maxdigits=rhs->digits+reqdigits-1;
    if (lhs->digits>maxdigits) maxdigits=lhs->digits;
    var1units=D2U(maxdigits)+2;
    /* allocate a guard unit above msu1 for REMAINDERNEAR  */
    if (!(op&DIVIDE)) var1units++;
    if ((var1units+1)*sizeof(Unit)>sizeof(varbuff)) {
      /* printf("malloc dvvar %ld units\n", var1units+1);  */
      varalloc=(Unit *)malloc((var1units+1)*sizeof(Unit));
      if (varalloc==NULL) {             /* hopeless -- abandon  */
        *status|=DEC_Insufficient_storage;
        break;}
      var1=varalloc;                    /* use the allocated space  */
      }

    /* Extend the lhs and rhs to full long subtraction length.  The lhs  */
    /* is truly extended into the var1 buffer, with 0 padding, so a  */
    /* subtract in place is always possible.  The rhs (var2) has  */
    /* virtual padding (implemented by decUnitAddSub).  */
    /* One guard unit was allocated above msu1 for rem=rem+rem in  */
    /* REMAINDERNEAR.  */
    msu1=var1+var1units-1;              /* msu of var1  */
    source=lhs->lsu+D2U(lhs->digits)-1; /* msu of input array  */
    for (target=msu1; source>=lhs->lsu; source--, target--) *target=*source;
    for (; target>=var1; target--) *target=0;

    /* rhs (var2) is left-aligned with var1 at the start  */
    var2ulen=var1units;                 /* rhs logical length (units)  */
    var2units=D2U(rhs->digits);         /* rhs actual length (units)  */
    var2=rhs->lsu;                      /* -> rhs array  */
    msu2=var2+var2units-1;              /* -> msu of var2 [never changes]  */
    /* now set up the variables which will be used for estimating the  */
    /* multiplication factor.  If these variables are not exact, add  */
    /* 1 to make sure that the multiplier is never overestimated.  */
    msu2plus=*msu2;                     /* it's value ..  */
    if (var2units>1) msu2plus++;        /* .. +1 if any more  */
    msu2pair=(eInt)*msu2*(DECDPUNMAX+1);/* top two pair ..  */
    if (var2units>1) {                  /* .. [else treat 2nd as 0]  */
      msu2pair+=*(msu2-1);              /* ..  */
      if (var2units>2) msu2pair++;      /* .. +1 if any more  */
      }

    /* The calculation is working in units, which may have leading zeros,  */
    /* but the exponent was calculated on the assumption that they are  */
    /* both left-aligned.  Adjust the exponent to compensate: add the  */
    /* number of leading zeros in var1 msu and subtract those in var2 msu.  */
    /* [This is actually done by counting the digits and negating, as  */
    /* lead1=DECDPUN-digits1, and similarly for lead2.]  */
    for (pow=&powers[1]; *msu1>=*pow; pow++) exponent--;
    for (pow=&powers[1]; *msu2>=*pow; pow++) exponent++;

    /* Now, if doing an integer divide or remainder, ensure that  */
    /* the result will be Unit-aligned.  To do this, shift the var1  */
    /* accumulator towards least if need be.  (It's much easier to  */
    /* do this now than to reassemble the residue afterwards, if  */
    /* doing a remainder.)  Also ensure the exponent is not negative.  */
    if (!(op&DIVIDE)) {
      Unit *u;                          /* work  */
      /* save the initial 'false' padding of var1, in digits  */
      var1initpad=(var1units-D2U(lhs->digits))*DECDPUN;
      /* Determine the shift to do.  */
      if (exponent<0) cut=-exponent;
       else cut=DECDPUN-exponent%DECDPUN;
      decShiftToLeast(var1, var1units, cut);
      exponent+=cut;                    /* maintain numerical value  */
      var1initpad-=cut;                 /* .. and reduce padding  */
      /* clean any most-significant units which were just emptied  */
      for (u=msu1; cut>=DECDPUN; cut-=DECDPUN, u--) *u=0;
      } /* align  */
     else { /* is DIVIDE  */
      maxexponent=lhs->exponent-rhs->exponent;    /* save  */
      /* optimization: if the first iteration will just produce 0,  */
      /* preadjust to skip it [valid for DIVIDE only]  */
      if (*msu1<*msu2) {
        var2ulen--;                     /* shift down  */
        exponent-=DECDPUN;              /* update the exponent  */
        }
      }

    /* ---- start the long-division loops ------------------------------  */
    accunits=0;                         /* no units accumulated yet  */
    accdigits=0;                        /* .. or digits  */
    accnext=acc+acclength-1;            /* -> msu of acc [NB: allows digits+1]  */
    for (;;) {                          /* outer forever loop  */
      thisunit=0;                       /* current unit assumed 0  */
      /* find the next unit  */
      for (;;) {                        /* inner forever loop  */
        /* strip leading zero units [from either pre-adjust or from  */
        /* subtract last time around].  Leave at least one unit.  */
        for (; *msu1==0 && msu1>var1; msu1--) var1units--;

        if (var1units<var2ulen) break;       /* var1 too low for subtract  */
        if (var1units==var2ulen) {           /* unit-by-unit compare needed  */
          /* compare the two numbers, from msu  */
          const Unit *pv1, *pv2;
          Unit v2;                           /* units to compare  */
          pv2=msu2;                          /* -> msu  */
          for (pv1=msu1; ; pv1--, pv2--) {
            /* v1=*pv1 -- always OK  */
            v2=0;                            /* assume in padding  */
            if (pv2>=var2) v2=*pv2;          /* in range  */
            if (*pv1!=v2) break;             /* no longer the same  */
            if (pv1==var1) break;            /* done; leave pv1 as is  */
            }
          /* here when all inspected or a difference seen  */
          if (*pv1<v2) break;                /* var1 too low to subtract  */
          if (*pv1==v2) {                    /* var1 == var2  */
            /* reach here if var1 and var2 are identical; subtraction  */
            /* would increase digit by one, and the residue will be 0 so  */
            /* the calculation is done; leave the loop with residue=0.  */
            thisunit++;                      /* as though subtracted  */
            *var1=0;                         /* set var1 to 0  */
            var1units=1;                     /* ..  */
            break;  /* from inner  */
            } /* var1 == var2  */
          /* *pv1>v2.  Prepare for real subtraction; the lengths are equal  */
          /* Estimate the multiplier (there's always a msu1-1)...  */
          /* Bring in two units of var2 to provide a good estimate.  */
          mult=(Int)(((eInt)*msu1*(DECDPUNMAX+1)+*(msu1-1))/msu2pair);
          } /* lengths the same  */


// Source: decNumber.cpp
// Lines 4252-4578
