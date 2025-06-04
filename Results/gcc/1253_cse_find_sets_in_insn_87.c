find_sets_in_insn (rtx_insn *insn, struct set **psets)
{
  struct set *sets = *psets;
  int n_sets = 0;
  rtx x = PATTERN (insn);

  if (GET_CODE (x) == SET)
    {
      /* Ignore SETs that are unconditional jumps.
	 They never need cse processing, so this does not hurt.
	 The reason is not efficiency but rather
	 so that we can test at the end for instructions
	 that have been simplified to unconditional jumps
	 and not be misled by unchanged instructions
	 that were unconditional jumps to begin with.  */
      if (SET_DEST (x) == pc_rtx
	  && GET_CODE (SET_SRC (x)) == LABEL_REF)
	;
      /* Don't count call-insns, (set (reg 0) (call ...)), as a set.
	 The hard function value register is used only once, to copy to
	 someplace else, so it isn't worth cse'ing.  */
      else if (GET_CODE (SET_SRC (x)) == CALL)
	;
      else
	sets[n_sets++].rtl = x;
    }
  else if (GET_CODE (x) == PARALLEL)
    {
      int i, lim = XVECLEN (x, 0);

      /* Go over the expressions of the PARALLEL in forward order, to
	 put them in the same order in the SETS array.  */
      for (i = 0; i < lim; i++)
	{
	  rtx y = XVECEXP (x, 0, i);
	  if (GET_CODE (y) == SET)
	    {
	      /* As above, we ignore unconditional jumps and call-insns and
		 ignore the result of apply_change_group.  */
	      if (SET_DEST (y) == pc_rtx
		  && GET_CODE (SET_SRC (y)) == LABEL_REF)
		;
	      else if (GET_CODE (SET_SRC (y)) == CALL)
		;
	      else
		sets[n_sets++].rtl = y;
	    }
	}
    }

  return n_sets;
}


// Source: cse.c
// Lines 4286-4337
