try_split (rtx pat, rtx_insn *trial, int last)
{
  rtx_insn *before, *after;
  rtx note;
  rtx_insn *seq, *tem;
  profile_probability probability;
  rtx_insn *insn_last, *insn;
  int njumps = 0;
  rtx_insn *call_insn = NULL;

  /* We're not good at redistributing frame information.  */
  if (RTX_FRAME_RELATED_P (trial))
    return trial;

  if (any_condjump_p (trial)
      && (note = find_reg_note (trial, REG_BR_PROB, 0)))
    split_branch_probability
      = profile_probability::from_reg_br_prob_note (XINT (note, 0));
  else
    split_branch_probability = profile_probability::uninitialized ();

  probability = split_branch_probability;

  seq = split_insns (pat, trial);

  split_branch_probability = profile_probability::uninitialized ();

  if (!seq)
    return trial;

  /* Avoid infinite loop if any insn of the result matches
     the original pattern.  */
  insn_last = seq;
  while (1)
    {
      if (INSN_P (insn_last)
	  && rtx_equal_p (PATTERN (insn_last), pat))
	return trial;
      if (!NEXT_INSN (insn_last))
	break;
      insn_last = NEXT_INSN (insn_last);
    }

  /* We will be adding the new sequence to the function.  The splitters
     may have introduced invalid RTL sharing, so unshare the sequence now.  */
  unshare_all_rtl_in_chain (seq);

  /* Mark labels and copy flags.  */
  for (insn = insn_last; insn ; insn = PREV_INSN (insn))
    {
      if (JUMP_P (insn))
	{
	  if (JUMP_P (trial))
	    CROSSING_JUMP_P (insn) = CROSSING_JUMP_P (trial);
	  mark_jump_label (PATTERN (insn), insn, 0);
	  njumps++;
	  if (probability.initialized_p ()
	      && any_condjump_p (insn)
	      && !find_reg_note (insn, REG_BR_PROB, 0))
	    {
	      /* We can preserve the REG_BR_PROB notes only if exactly
		 one jump is created, otherwise the machine description
		 is responsible for this step using
		 split_branch_probability variable.  */
	      gcc_assert (njumps == 1);
	      add_reg_br_prob_note (insn, probability);
	    }
	}
    }

  /* If we are splitting a CALL_INSN, look for the CALL_INSN
     in SEQ and copy any additional information across.  */
  if (CALL_P (trial))
    {
      for (insn = insn_last; insn ; insn = PREV_INSN (insn))
	if (CALL_P (insn))
	  {
	    gcc_assert (call_insn == NULL_RTX);
	    call_insn = insn;

	    /* Add the old CALL_INSN_FUNCTION_USAGE to whatever the
	       target may have explicitly specified.  */
	    rtx *p = &CALL_INSN_FUNCTION_USAGE (insn);
	    while (*p)
	      p = &XEXP (*p, 1);
	    *p = CALL_INSN_FUNCTION_USAGE (trial);

	    /* If the old call was a sibling call, the new one must
	       be too.  */
	    SIBLING_CALL_P (insn) = SIBLING_CALL_P (trial);
	  }
    }

  /* Copy notes, particularly those related to the CFG.  */
  for (note = REG_NOTES (trial); note; note = XEXP (note, 1))
    {
      switch (REG_NOTE_KIND (note))
	{
	case REG_EH_REGION:
	  copy_reg_eh_region_note_backward (note, insn_last, NULL);
	  break;

	case REG_NORETURN:
	case REG_SETJMP:
	case REG_TM:
	case REG_CALL_NOCF_CHECK:
	case REG_CALL_ARG_LOCATION:
	  for (insn = insn_last; insn != NULL_RTX; insn = PREV_INSN (insn))
	    {
	      if (CALL_P (insn))
		add_reg_note (insn, REG_NOTE_KIND (note), XEXP (note, 0));
	    }
	  break;

	case REG_NON_LOCAL_GOTO:
	case REG_LABEL_TARGET:
	  for (insn = insn_last; insn != NULL_RTX; insn = PREV_INSN (insn))
	    {
	      if (JUMP_P (insn))
		add_reg_note (insn, REG_NOTE_KIND (note), XEXP (note, 0));
	    }
	  break;

	case REG_INC:
	  if (!AUTO_INC_DEC)
	    break;

	  for (insn = insn_last; insn != NULL_RTX; insn = PREV_INSN (insn))
	    {
	      rtx reg = XEXP (note, 0);
	      if (!FIND_REG_INC_NOTE (insn, reg)
		  && find_auto_inc (PATTERN (insn), reg))
		add_reg_note (insn, REG_INC, reg);
	    }
	  break;

	case REG_ARGS_SIZE:
	  fixup_args_size_notes (NULL, insn_last, get_args_size (note));
	  break;

	case REG_CALL_DECL:
	  gcc_assert (call_insn != NULL_RTX);
	  add_reg_note (call_insn, REG_NOTE_KIND (note), XEXP (note, 0));
	  break;

	default:
	  break;
	}
    }

  /* If there are LABELS inside the split insns increment the
     usage count so we don't delete the label.  */
  if (INSN_P (trial))
    {
      insn = insn_last;
      while (insn != NULL_RTX)
	{
	  /* JUMP_P insns have already been "marked" above.  */
	  if (NONJUMP_INSN_P (insn))
	    mark_label_nuses (PATTERN (insn));

	  insn = PREV_INSN (insn);
	}
    }

  before = PREV_INSN (trial);
  after = NEXT_INSN (trial);

  emit_insn_after_setloc (seq, trial, INSN_LOCATION (trial));

  delete_insn (trial);

  /* Recursively call try_split for each new insn created; by the
     time control returns here that insn will be fully split, so
     set LAST and continue from the insn after the one returned.
     We can't use next_active_insn here since AFTER may be a note.
     Ignore deleted insns, which can be occur if not optimizing.  */
  for (tem = NEXT_INSN (before); tem != after; tem = NEXT_INSN (tem))
    if (! tem->deleted () && INSN_P (tem))
      tem = try_split (PATTERN (tem), tem, 1);

  /* Return either the first or the last insn, depending on which was
     requested.  */
  return last
    ? (after ? PREV_INSN (after) : get_last_insn ())
    : NEXT_INSN (before);
}


// Source: emit-rtl.c
// Lines 3813-3999
