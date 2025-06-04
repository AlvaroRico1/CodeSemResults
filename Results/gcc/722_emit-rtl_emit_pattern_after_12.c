emit_pattern_after (rtx pattern, rtx_insn *after, bool skip_debug_insns,
		    rtx_insn *(*make_raw) (rtx))
{
  rtx_insn *prev = after;

  if (skip_debug_insns)
    while (DEBUG_INSN_P (prev))
      prev = PREV_INSN (prev);

  if (INSN_P (prev))
    return emit_pattern_after_setloc (pattern, after, INSN_LOCATION (prev),
				      make_raw);
  else
    return emit_pattern_after_noloc (pattern, after, NULL, make_raw);
}


// Source: emit-rtl.c
// Lines 4852-4866
