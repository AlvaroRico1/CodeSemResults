set_new_first_and_last_insn (rtx_insn *first, rtx_insn *last)
{
  rtx_insn *insn;

  set_first_insn (first);
  set_last_insn (last);
  cur_insn_uid = 0;

  if (param_min_nondebug_insn_uid || MAY_HAVE_DEBUG_INSNS)
    {
      int debug_count = 0;

      cur_insn_uid = param_min_nondebug_insn_uid - 1;
      cur_debug_insn_uid = 0;

      for (insn = first; insn; insn = NEXT_INSN (insn))
	if (INSN_UID (insn) < param_min_nondebug_insn_uid)
	  cur_debug_insn_uid = MAX (cur_debug_insn_uid, INSN_UID (insn));
	else
	  {
	    cur_insn_uid = MAX (cur_insn_uid, INSN_UID (insn));
	    if (DEBUG_INSN_P (insn))
	      debug_count++;
	  }

      if (debug_count)
	cur_debug_insn_uid = param_min_nondebug_insn_uid + debug_count;
      else
	cur_debug_insn_uid++;
    }
  else
    for (insn = first; insn; insn = NEXT_INSN (insn))
      cur_insn_uid = MAX (cur_insn_uid, INSN_UID (insn));

  cur_insn_uid++;
}


// Source: emit-rtl.c
// Lines 2711-2746
