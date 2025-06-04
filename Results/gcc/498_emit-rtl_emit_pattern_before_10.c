emit_pattern_before (rtx pattern, rtx_insn *before, bool skip_debug_insns,
		     bool insnp, rtx_insn *(*make_raw) (rtx))
{
  rtx_insn *next = before;

  if (skip_debug_insns)
    while (DEBUG_INSN_P (next))
      next = PREV_INSN (next);

  if (INSN_P (next))
    return emit_pattern_before_setloc (pattern, before, INSN_LOCATION (next),
				       insnp, make_raw);
  else
    return emit_pattern_before_noloc (pattern, before,
				      insnp ? before : NULL,
                                      NULL, make_raw);
}


// Source: emit-rtl.c
// Lines 4966-4982
