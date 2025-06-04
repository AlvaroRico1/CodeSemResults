fold_rtx (rtx x, rtx_insn *insn)
{
  enum rtx_code code;
  machine_mode mode;
  const char *fmt;
  int i;
  rtx new_rtx = 0;
  int changed = 0;
  poly_int64 xval;

  /* Operands of X.  */
  /* Workaround -Wmaybe-uninitialized false positive during
     profiledbootstrap by initializing them.  */
  rtx folded_arg0 = NULL_RTX;
  rtx folded_arg1 = NULL_RTX;

  /* Constant equivalents of first three operands of X;
     0 when no such equivalent is known.  */
  rtx const_arg0;
  rtx const_arg1;
  rtx const_arg2;

  /* The mode of the first operand of X.  We need this for sign and zero
     extends.  */
  machine_mode mode_arg0;

  if (x == 0)
    return x;

  /* Try to perform some initial simplifications on X.  */
  code = GET_CODE (x);
  switch (code)
    {
    case MEM:
    case SUBREG:
    /* The first operand of a SIGN/ZERO_EXTRACT has a different meaning
       than it would in other contexts.  Basically its mode does not
       signify the size of the object read.  That information is carried
       by size operand.    If we happen to have a MEM of the appropriate
       mode in our tables with a constant value we could simplify the
       extraction incorrectly if we allowed substitution of that value
       for the MEM.   */
    case ZERO_EXTRACT:
    case SIGN_EXTRACT:
      if ((new_rtx = equiv_constant (x)) != NULL_RTX)
        return new_rtx;
      return x;

    case CONST:
    CASE_CONST_ANY:
    case SYMBOL_REF:
    case LABEL_REF:
    case REG:
    case PC:
      /* No use simplifying an EXPR_LIST
	 since they are used only for lists of args
	 in a function call's REG_EQUAL note.  */
    case EXPR_LIST:
      return x;

    case CC0:
      return prev_insn_cc0;

    case ASM_OPERANDS:
      if (insn)
	{
	  for (i = ASM_OPERANDS_INPUT_LENGTH (x) - 1; i >= 0; i--)
	    validate_change (insn, &ASM_OPERANDS_INPUT (x, i),
			     fold_rtx (ASM_OPERANDS_INPUT (x, i), insn), 0);
	}
      return x;

    case CALL:
      if (NO_FUNCTION_CSE && CONSTANT_P (XEXP (XEXP (x, 0), 0)))
	return x;
      break;

    /* Anything else goes through the loop below.  */
    default:
      break;
    }

  mode = GET_MODE (x);
  const_arg0 = 0;
  const_arg1 = 0;
  const_arg2 = 0;
  mode_arg0 = VOIDmode;

  /* Try folding our operands.
     Then see which ones have constant values known.  */

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    if (fmt[i] == 'e')
      {
	rtx folded_arg = XEXP (x, i), const_arg;
	machine_mode mode_arg = GET_MODE (folded_arg);

	switch (GET_CODE (folded_arg))
	  {
	  case MEM:
	  case REG:
	  case SUBREG:
	    const_arg = equiv_constant (folded_arg);
	    break;

	  case CONST:
	  CASE_CONST_ANY:
	  case SYMBOL_REF:
	  case LABEL_REF:
	    const_arg = folded_arg;
	    break;

	  case CC0:
	    /* The cc0-user and cc0-setter may be in different blocks if
	       the cc0-setter potentially traps.  In that case PREV_INSN_CC0
	       will have been cleared as we exited the block with the
	       setter.

	       While we could potentially track cc0 in this case, it just
	       doesn't seem to be worth it given that cc0 targets are not
	       terribly common or important these days and trapping math
	       is rarely used.  The combination of those two conditions
	       necessary to trip this situation is exceedingly rare in the
	       real world.  */
	    if (!prev_insn_cc0)
	      {
		const_arg = NULL_RTX;
	      }
	    else
	      {
		folded_arg = prev_insn_cc0;
		mode_arg = prev_insn_cc0_mode;
		const_arg = equiv_constant (folded_arg);
	      }
	    break;

	  default:
	    folded_arg = fold_rtx (folded_arg, insn);
	    const_arg = equiv_constant (folded_arg);
	    break;
	  }

	/* For the first three operands, see if the operand
	   is constant or equivalent to a constant.  */
	switch (i)
	  {
	  case 0:
	    folded_arg0 = folded_arg;
	    const_arg0 = const_arg;
	    mode_arg0 = mode_arg;
	    break;
	  case 1:
	    folded_arg1 = folded_arg;
	    const_arg1 = const_arg;
	    break;
	  case 2:
	    const_arg2 = const_arg;
	    break;
	  }

	/* Pick the least expensive of the argument and an equivalent constant
	   argument.  */
	if (const_arg != 0
	    && const_arg != folded_arg
	    && (COST_IN (const_arg, mode_arg, code, i)
		<= COST_IN (folded_arg, mode_arg, code, i))

	    /* It's not safe to substitute the operand of a conversion
	       operator with a constant, as the conversion's identity
	       depends upon the mode of its operand.  This optimization
	       is handled by the call to simplify_unary_operation.  */
	    && (GET_RTX_CLASS (code) != RTX_UNARY
		|| GET_MODE (const_arg) == mode_arg0
		|| (code != ZERO_EXTEND
		    && code != SIGN_EXTEND
		    && code != TRUNCATE
		    && code != FLOAT_TRUNCATE
		    && code != FLOAT_EXTEND
		    && code != FLOAT
		    && code != FIX
		    && code != UNSIGNED_FLOAT
		    && code != UNSIGNED_FIX)))
	  folded_arg = const_arg;

	if (folded_arg == XEXP (x, i))
	  continue;

	if (insn == NULL_RTX && !changed)
	  x = copy_rtx (x);
	changed = 1;
	validate_unshare_change (insn, &XEXP (x, i), folded_arg, 1);
      }

  if (changed)
    {
      /* Canonicalize X if necessary, and keep const_argN and folded_argN
	 consistent with the order in X.  */
      if (canonicalize_change_group (insn, x))
	{
	  std::swap (const_arg0, const_arg1);
	  std::swap (folded_arg0, folded_arg1);
	}

      apply_change_group ();
    }

  /* If X is an arithmetic operation, see if we can simplify it.  */

  switch (GET_RTX_CLASS (code))
    {
    case RTX_UNARY:
      {
	/* We can't simplify extension ops unless we know the
	   original mode.  */
	if ((code == ZERO_EXTEND || code == SIGN_EXTEND)
	    && mode_arg0 == VOIDmode)
	  break;

	new_rtx = simplify_unary_operation (code, mode,
					    const_arg0 ? const_arg0 : folded_arg0,
					    mode_arg0);
      }
      break;

    case RTX_COMPARE:
    case RTX_COMM_COMPARE:
      /* See what items are actually being compared and set FOLDED_ARG[01]
	 to those values and CODE to the actual comparison code.  If any are
	 constant, set CONST_ARG0 and CONST_ARG1 appropriately.  We needn't
	 do anything if both operands are already known to be constant.  */

      /* ??? Vector mode comparisons are not supported yet.  */
      if (VECTOR_MODE_P (mode))
	break;

      if (const_arg0 == 0 || const_arg1 == 0)
	{
	  struct table_elt *p0, *p1;
	  rtx true_rtx, false_rtx;
	  machine_mode mode_arg1;

	  if (SCALAR_FLOAT_MODE_P (mode))
	    {
#ifdef FLOAT_STORE_FLAG_VALUE
	      true_rtx = (const_double_from_real_value
			  (FLOAT_STORE_FLAG_VALUE (mode), mode));
#else
	      true_rtx = NULL_RTX;
#endif
	      false_rtx = CONST0_RTX (mode);
	    }
	  else
	    {
	      true_rtx = const_true_rtx;
	      false_rtx = const0_rtx;
	    }

	  code = find_comparison_args (code, &folded_arg0, &folded_arg1,
				       &mode_arg0, &mode_arg1);

	  /* If the mode is VOIDmode or a MODE_CC mode, we don't know
	     what kinds of things are being compared, so we can't do
	     anything with this comparison.  */

	  if (mode_arg0 == VOIDmode || GET_MODE_CLASS (mode_arg0) == MODE_CC)
	    break;

	  const_arg0 = equiv_constant (folded_arg0);
	  const_arg1 = equiv_constant (folded_arg1);

	  /* If we do not now have two constants being compared, see
	     if we can nevertheless deduce some things about the
	     comparison.  */
	  if (const_arg0 == 0 || const_arg1 == 0)
	    {
	      if (const_arg1 != NULL)
		{
		  rtx cheapest_simplification;
		  int cheapest_cost;
		  rtx simp_result;
		  struct table_elt *p;

		  /* See if we can find an equivalent of folded_arg0
		     that gets us a cheaper expression, possibly a
		     constant through simplifications.  */
		  p = lookup (folded_arg0, SAFE_HASH (folded_arg0, mode_arg0),
			      mode_arg0);

		  if (p != NULL)
		    {
		      cheapest_simplification = x;
		      cheapest_cost = COST (x, mode);

		      for (p = p->first_same_value; p != NULL; p = p->next_same_value)
			{
			  int cost;

			  /* If the entry isn't valid, skip it.  */
			  if (! exp_equiv_p (p->exp, p->exp, 1, false))
			    continue;

			  /* Try to simplify using this equivalence.  */
			  simp_result
			    = simplify_relational_operation (code, mode,
							     mode_arg0,
							     p->exp,
							     const_arg1);

			  if (simp_result == NULL)
			    continue;

			  cost = COST (simp_result, mode);
			  if (cost < cheapest_cost)
			    {
			      cheapest_cost = cost;
			      cheapest_simplification = simp_result;
			    }
			}

		      /* If we have a cheaper expression now, use that
			 and try folding it further, from the top.  */
		      if (cheapest_simplification != x)
			return fold_rtx (copy_rtx (cheapest_simplification),
					 insn);
		    }
		}

	      /* See if the two operands are the same.  */

	      if ((REG_P (folded_arg0)
		   && REG_P (folded_arg1)
		   && (REG_QTY (REGNO (folded_arg0))
		       == REG_QTY (REGNO (folded_arg1))))
		  || ((p0 = lookup (folded_arg0,
				    SAFE_HASH (folded_arg0, mode_arg0),
				    mode_arg0))
		      && (p1 = lookup (folded_arg1,
				       SAFE_HASH (folded_arg1, mode_arg0),
				       mode_arg0))
		      && p0->first_same_value == p1->first_same_value))
		folded_arg1 = folded_arg0;

	      /* If FOLDED_ARG0 is a register, see if the comparison we are
		 doing now is either the same as we did before or the reverse
		 (we only check the reverse if not floating-point).  */
	      else if (REG_P (folded_arg0))
		{
		  int qty = REG_QTY (REGNO (folded_arg0));

		  if (REGNO_QTY_VALID_P (REGNO (folded_arg0)))
		    {
		      struct qty_table_elem *ent = &qty_table[qty];

		      if ((comparison_dominates_p (ent->comparison_code, code)
			   || (! FLOAT_MODE_P (mode_arg0)
			       && comparison_dominates_p (ent->comparison_code,
						          reverse_condition (code))))
			  && (rtx_equal_p (ent->comparison_const, folded_arg1)
			      || (const_arg1
				  && rtx_equal_p (ent->comparison_const,
						  const_arg1))
			      || (REG_P (folded_arg1)
				  && (REG_QTY (REGNO (folded_arg1)) == ent->comparison_qty))))
			{
			  if (comparison_dominates_p (ent->comparison_code, code))
			    {
			      if (true_rtx)
				return true_rtx;
			      else
				break;
			    }
			  else
			    return false_rtx;
			}
		    }
		}
	    }
	}

      /* If we are comparing against zero, see if the first operand is
	 equivalent to an IOR with a constant.  If so, we may be able to
	 determine the result of this comparison.  */
      if (const_arg1 == const0_rtx && !const_arg0)
	{
	  rtx y = lookup_as_function (folded_arg0, IOR);
	  rtx inner_const;

	  if (y != 0
	      && (inner_const = equiv_constant (XEXP (y, 1))) != 0
	      && CONST_INT_P (inner_const)
	      && INTVAL (inner_const) != 0)
	    folded_arg0 = gen_rtx_IOR (mode_arg0, XEXP (y, 0), inner_const);
	}

      {
	rtx op0 = const_arg0 ? const_arg0 : copy_rtx (folded_arg0);
	rtx op1 = const_arg1 ? const_arg1 : copy_rtx (folded_arg1);
	new_rtx = simplify_relational_operation (code, mode, mode_arg0,
						 op0, op1);
      }
      break;

    case RTX_BIN_ARITH:
    case RTX_COMM_ARITH:
      switch (code)
	{
	case PLUS:
	  /* If the second operand is a LABEL_REF, see if the first is a MINUS
	     with that LABEL_REF as its second operand.  If so, the result is
	     the first operand of that MINUS.  This handles switches with an
	     ADDR_DIFF_VEC table.  */
	  if (const_arg1 && GET_CODE (const_arg1) == LABEL_REF)
	    {
	      rtx y
		= GET_CODE (folded_arg0) == MINUS ? folded_arg0
		: lookup_as_function (folded_arg0, MINUS);

	      if (y != 0 && GET_CODE (XEXP (y, 1)) == LABEL_REF
		  && label_ref_label (XEXP (y, 1)) == label_ref_label (const_arg1))
		return XEXP (y, 0);

	      /* Now try for a CONST of a MINUS like the above.  */
	      if ((y = (GET_CODE (folded_arg0) == CONST ? folded_arg0
			: lookup_as_function (folded_arg0, CONST))) != 0
		  && GET_CODE (XEXP (y, 0)) == MINUS
		  && GET_CODE (XEXP (XEXP (y, 0), 1)) == LABEL_REF
		  && label_ref_label (XEXP (XEXP (y, 0), 1)) == label_ref_label (const_arg1))
		return XEXP (XEXP (y, 0), 0);
	    }

	  /* Likewise if the operands are in the other order.  */
	  if (const_arg0 && GET_CODE (const_arg0) == LABEL_REF)
	    {
	      rtx y
		= GET_CODE (folded_arg1) == MINUS ? folded_arg1
		: lookup_as_function (folded_arg1, MINUS);

	      if (y != 0 && GET_CODE (XEXP (y, 1)) == LABEL_REF
		  && label_ref_label (XEXP (y, 1)) == label_ref_label (const_arg0))
		return XEXP (y, 0);

	      /* Now try for a CONST of a MINUS like the above.  */
	      if ((y = (GET_CODE (folded_arg1) == CONST ? folded_arg1
			: lookup_as_function (folded_arg1, CONST))) != 0
		  && GET_CODE (XEXP (y, 0)) == MINUS
		  && GET_CODE (XEXP (XEXP (y, 0), 1)) == LABEL_REF
		  && label_ref_label (XEXP (XEXP (y, 0), 1)) == label_ref_label (const_arg0))
		return XEXP (XEXP (y, 0), 0);
	    }

	  /* If second operand is a register equivalent to a negative
	     CONST_INT, see if we can find a register equivalent to the
	     positive constant.  Make a MINUS if so.  Don't do this for
	     a non-negative constant since we might then alternate between
	     choosing positive and negative constants.  Having the positive
	     constant previously-used is the more common case.  Be sure
	     the resulting constant is non-negative; if const_arg1 were
	     the smallest negative number this would overflow: depending
	     on the mode, this would either just be the same value (and
	     hence not save anything) or be incorrect.  */
	  if (const_arg1 != 0 && CONST_INT_P (const_arg1)
	      && INTVAL (const_arg1) < 0
	      /* This used to test

	         -INTVAL (const_arg1) >= 0

		 But The Sun V5.0 compilers mis-compiled that test.  So
		 instead we test for the problematic value in a more direct
		 manner and hope the Sun compilers get it correct.  */
	      && INTVAL (const_arg1) !=
	        (HOST_WIDE_INT_1 << (HOST_BITS_PER_WIDE_INT - 1))
	      && REG_P (folded_arg1))
	    {
	      rtx new_const = GEN_INT (-INTVAL (const_arg1));
	      struct table_elt *p
		= lookup (new_const, SAFE_HASH (new_const, mode), mode);

	      if (p)
		for (p = p->first_same_value; p; p = p->next_same_value)
		  if (REG_P (p->exp))
		    return simplify_gen_binary (MINUS, mode, folded_arg0,
						canon_reg (p->exp, NULL));
	    }


// Source: cse.c
// Lines 3117-3600
