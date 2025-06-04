next_insn_no_annul (rtx_insn *insn)
{
  if (insn)
    {
      /* If INSN is an annulled branch, skip any insns from the target
	 of the branch.  */
      if (JUMP_P (insn)
	  && INSN_ANNULLED_BRANCH_P (insn)
	  && NEXT_INSN (PREV_INSN (insn)) != insn)
	{
	  rtx_insn *next = NEXT_INSN (insn);

	  while ((NONJUMP_INSN_P (next) || JUMP_P (next) || CALL_P (next))
		 && INSN_FROM_TARGET_P (next))
	    {
	      insn = next;
	      next = NEXT_INSN (insn);
	    }
	}


// Source: resource.c
// Lines 163-181
