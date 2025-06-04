find_shift_sequence (poly_int64 access_size,
		     store_info *store_info,
		     machine_mode read_mode,
		     poly_int64 shift, bool speed, bool require_cst)
{
  machine_mode store_mode = GET_MODE (store_info->mem);
  scalar_int_mode new_mode;
  rtx read_reg = NULL;

  /* Some machines like the x86 have shift insns for each size of
     operand.  Other machines like the ppc or the ia-64 may only have
     shift insns that shift values within 32 or 64 bit registers.
     This loop tries to find the smallest shift insn that will right
     justify the value we want to read but is available in one insn on
     the machine.  */

  opt_scalar_int_mode new_mode_iter;
  FOR_EACH_MODE_IN_CLASS (new_mode_iter, MODE_INT)
    {
      rtx target, new_reg, new_lhs;
      rtx_insn *shift_seq, *insn;
      int cost;

      new_mode = new_mode_iter.require ();
      if (GET_MODE_BITSIZE (new_mode) > BITS_PER_WORD)
	break;
      if (maybe_lt (GET_MODE_SIZE (new_mode), access_size))
	continue;

      /* If a constant was stored into memory, try to simplify it here,
	 otherwise the cost of the shift might preclude this optimization
	 e.g. at -Os, even when no actual shift will be needed.  */
      if (store_info->const_rhs)
	{
	  poly_uint64 byte = subreg_lowpart_offset (new_mode, store_mode);
	  rtx ret = simplify_subreg (new_mode, store_info->const_rhs,
				     store_mode, byte);
	  if (ret && CONSTANT_P (ret))
	    {
	      rtx shift_rtx = gen_int_shift_amount (new_mode, shift);
	      ret = simplify_const_binary_operation (LSHIFTRT, new_mode,
						     ret, shift_rtx);
	      if (ret && CONSTANT_P (ret))
		{
		  byte = subreg_lowpart_offset (read_mode, new_mode);
		  ret = simplify_subreg (read_mode, ret, new_mode, byte);
		  if (ret && CONSTANT_P (ret)
		      && (set_src_cost (ret, read_mode, speed)
			  <= COSTS_N_INSNS (1)))
		    return ret;
		}
	    }
	}

      if (require_cst)
	return NULL_RTX;

      /* Try a wider mode if truncating the store mode to NEW_MODE
	 requires a real instruction.  */
      if (maybe_lt (GET_MODE_SIZE (new_mode), GET_MODE_SIZE (store_mode))
	  && !TRULY_NOOP_TRUNCATION_MODES_P (new_mode, store_mode))
	continue;

      /* Also try a wider mode if the necessary punning is either not
	 desirable or not possible.  */
      if (!CONSTANT_P (store_info->rhs)
	  && !targetm.modes_tieable_p (new_mode, store_mode))
	continue;

      new_reg = gen_reg_rtx (new_mode);

      start_sequence ();

      /* In theory we could also check for an ashr.  Ian Taylor knows
	 of one dsp where the cost of these two was not the same.  But
	 this really is a rare case anyway.  */
      target = expand_binop (new_mode, lshr_optab, new_reg,
			     gen_int_shift_amount (new_mode, shift),
			     new_reg, 1, OPTAB_DIRECT);

      shift_seq = get_insns ();
      end_sequence ();

      if (target != new_reg || shift_seq == NULL)
	continue;

      cost = 0;
      for (insn = shift_seq; insn != NULL_RTX; insn = NEXT_INSN (insn))
	if (INSN_P (insn))
	  cost += insn_cost (insn, speed);

      /* The computation up to here is essentially independent
	 of the arguments and could be precomputed.  It may
	 not be worth doing so.  We could precompute if
	 worthwhile or at least cache the results.  The result
	 technically depends on both SHIFT and ACCESS_SIZE,
	 but in practice the answer will depend only on ACCESS_SIZE.  */

      if (cost > COSTS_N_INSNS (1))
	continue;

      new_lhs = extract_low_bits (new_mode, store_mode,
				  copy_rtx (store_info->rhs));
      if (new_lhs == NULL_RTX)
	continue;

      /* We found an acceptable shift.  Generate a move to
	 take the value from the store and put it into the
	 shift pseudo, then shift it, then generate another
	 move to put in into the target of the read.  */
      emit_move_insn (new_reg, new_lhs);
      emit_insn (shift_seq);
      read_reg = extract_low_bits (read_mode, new_mode, new_reg);
      break;
    }


// Source: dse.c
// Lines 1714-1828
