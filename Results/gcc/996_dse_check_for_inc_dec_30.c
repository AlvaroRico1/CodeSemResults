check_for_inc_dec (rtx_insn *insn)
{
  insn_info_type insn_info;
  rtx note;

  insn_info.insn = insn;
  insn_info.fixed_regs_live = NULL;
  note = find_reg_note (insn, REG_INC, NULL_RTX);
  if (note)
    return for_each_inc_dec (PATTERN (insn), emit_inc_dec_insn_before,
			     &insn_info) == 0;

  /* Punt on stack pushes, those don't have REG_INC notes and we are
     unprepared to deal with distribution of REG_ARGS_SIZE notes etc.  */
  subrtx_iterator::array_type array;
  FOR_EACH_SUBRTX (iter, array, PATTERN (insn), NONCONST)
    {
      const_rtx x = *iter;
      if (GET_RTX_CLASS (GET_CODE (x)) == RTX_AUTOINC)
	return false;
    }

  return true;
}


// Source: dse.c
// Lines 869-892
