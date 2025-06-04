cse_change_cc_mode_insns (rtx_insn *start, rtx_insn *end, rtx newreg)
{
  rtx_insn *insn;

  for (insn = start; insn != end; insn = NEXT_INSN (insn))
    {
      if (! INSN_P (insn))
	continue;

      if (reg_set_p (newreg, insn))
	return;

      cse_change_cc_mode_insn (insn, newreg);
    }
}


// Source: cse.c
// Lines 7262-7276
