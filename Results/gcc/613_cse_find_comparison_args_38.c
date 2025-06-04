find_comparison_args (enum rtx_code code, rtx *parg1, rtx *parg2,
		      machine_mode *pmode1, machine_mode *pmode2)
{
  rtx arg1, arg2;
  hash_set<rtx> *visited = NULL;
  /* Set nonzero when we find something of interest.  */
  rtx x = NULL;

  arg1 = *parg1, arg2 = *parg2;

  /* If ARG2 is const0_rtx, see what ARG1 is equivalent to.  */

  while (arg2 == CONST0_RTX (GET_MODE (arg1)))
    {
      int reverse_code = 0;
      struct table_elt *p = 0;

      /* Remember state from previous iteration.  */
      if (x)
	{
	  if (!visited)
	    visited = new hash_set<rtx>;
	  visited->add (x);
	  x = 0;
	}

      /* If arg1 is a COMPARE, extract the comparison arguments from it.
	 On machines with CC0, this is the only case that can occur, since
	 fold_rtx will return the COMPARE or item being compared with zero
	 when given CC0.  */

      if (GET_CODE (arg1) == COMPARE && arg2 == const0_rtx)
	x = arg1;

      /* If ARG1 is a comparison operator and CODE is testing for
	 STORE_FLAG_VALUE, get the inner arguments.  */

      else if (COMPARISON_P (arg1))
	{
#ifdef FLOAT_STORE_FLAG_VALUE
	  REAL_VALUE_TYPE fsfv;
#endif

	  if (code == NE
	      || (GET_MODE_CLASS (GET_MODE (arg1)) == MODE_INT
		  && code == LT && STORE_FLAG_VALUE == -1)
#ifdef FLOAT_STORE_FLAG_VALUE
	      || (SCALAR_FLOAT_MODE_P (GET_MODE (arg1))
		  && (fsfv = FLOAT_STORE_FLAG_VALUE (GET_MODE (arg1)),
		      REAL_VALUE_NEGATIVE (fsfv)))
#endif
	      )
	    x = arg1;
	  else if (code == EQ
		   || (GET_MODE_CLASS (GET_MODE (arg1)) == MODE_INT
		       && code == GE && STORE_FLAG_VALUE == -1)
#ifdef FLOAT_STORE_FLAG_VALUE
		   || (SCALAR_FLOAT_MODE_P (GET_MODE (arg1))
		       && (fsfv = FLOAT_STORE_FLAG_VALUE (GET_MODE (arg1)),
			   REAL_VALUE_NEGATIVE (fsfv)))
#endif
		   )
	    x = arg1, reverse_code = 1;
	}

      /* ??? We could also check for

	 (ne (and (eq (...) (const_int 1))) (const_int 0))

	 and related forms, but let's wait until we see them occurring.  */

      if (x == 0)
	/* Look up ARG1 in the hash table and see if it has an equivalence
	   that lets us see what is being compared.  */
	p = lookup (arg1, SAFE_HASH (arg1, GET_MODE (arg1)), GET_MODE (arg1));
      if (p)
	{
	  p = p->first_same_value;

	  /* If what we compare is already known to be constant, that is as
	     good as it gets.
	     We need to break the loop in this case, because otherwise we
	     can have an infinite loop when looking at a reg that is known
	     to be a constant which is the same as a comparison of a reg
	     against zero which appears later in the insn stream, which in
	     turn is constant and the same as the comparison of the first reg
	     against zero...  */
	  if (p->is_const)
	    break;
	}

      for (; p; p = p->next_same_value)
	{
	  machine_mode inner_mode = GET_MODE (p->exp);
#ifdef FLOAT_STORE_FLAG_VALUE
	  REAL_VALUE_TYPE fsfv;
#endif

	  /* If the entry isn't valid, skip it.  */
	  if (! exp_equiv_p (p->exp, p->exp, 1, false))
	    continue;

	  /* If it's a comparison we've used before, skip it.  */
	  if (visited && visited->contains (p->exp))
	    continue;

	  if (GET_CODE (p->exp) == COMPARE
	      /* Another possibility is that this machine has a compare insn
		 that includes the comparison code.  In that case, ARG1 would
		 be equivalent to a comparison operation that would set ARG1 to
		 either STORE_FLAG_VALUE or zero.  If this is an NE operation,
		 ORIG_CODE is the actual comparison being done; if it is an EQ,
		 we must reverse ORIG_CODE.  On machine with a negative value
		 for STORE_FLAG_VALUE, also look at LT and GE operations.  */
	      || ((code == NE
		   || (code == LT
		       && val_signbit_known_set_p (inner_mode,
						   STORE_FLAG_VALUE))
#ifdef FLOAT_STORE_FLAG_VALUE
		   || (code == LT
		       && SCALAR_FLOAT_MODE_P (inner_mode)
		       && (fsfv = FLOAT_STORE_FLAG_VALUE (GET_MODE (arg1)),
			   REAL_VALUE_NEGATIVE (fsfv)))
#endif
		   )
		  && COMPARISON_P (p->exp)))
	    {
	      x = p->exp;
	      break;
	    }
	  else if ((code == EQ
		    || (code == GE
			&& val_signbit_known_set_p (inner_mode,
						    STORE_FLAG_VALUE))
#ifdef FLOAT_STORE_FLAG_VALUE
		    || (code == GE
			&& SCALAR_FLOAT_MODE_P (inner_mode)
			&& (fsfv = FLOAT_STORE_FLAG_VALUE (GET_MODE (arg1)),
			    REAL_VALUE_NEGATIVE (fsfv)))
#endif
		    )
		   && COMPARISON_P (p->exp))
	    {
	      reverse_code = 1;
	      x = p->exp;
	      break;
	    }

	  /* If this non-trapping address, e.g. fp + constant, the
	     equivalent is a better operand since it may let us predict
	     the value of the comparison.  */
	  else if (!rtx_addr_can_trap_p (p->exp))
	    {
	      arg1 = p->exp;
	      continue;
	    }
	}

      /* If we didn't find a useful equivalence for ARG1, we are done.
	 Otherwise, set up for the next iteration.  */
      if (x == 0)
	break;

      /* If we need to reverse the comparison, make sure that is
	 possible -- we can't necessarily infer the value of GE from LT
	 with floating-point operands.  */
      if (reverse_code)
	{
	  enum rtx_code reversed = reversed_comparison_code (x, NULL);
	  if (reversed == UNKNOWN)
	    break;
	  else
	    code = reversed;
	}
      else if (COMPARISON_P (x))
	code = GET_CODE (x);
      arg1 = XEXP (x, 0), arg2 = XEXP (x, 1);
    }


// Source: cse.c
// Lines 2915-3092
