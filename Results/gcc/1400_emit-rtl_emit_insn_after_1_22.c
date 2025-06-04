emit_insn_after_1 (rtx_insn *first, rtx_insn *after, basic_block bb)
{
  rtx_insn *last;
  rtx_insn *after_after;
  if (!bb && !BARRIER_P (after))
    bb = BLOCK_FOR_INSN (after);

  if (bb)
    {
      df_set_bb_dirty (bb);
      for (last = first; NEXT_INSN (last); last = NEXT_INSN (last))
	if (!BARRIER_P (last))
	  {
	    set_block_for_insn (last, bb);
	    df_insn_rescan (last);
	  }
      if (!BARRIER_P (last))
	{
	  set_block_for_insn (last, bb);
	  df_insn_rescan (last);
	}
      if (BB_END (bb) == after)
	BB_END (bb) = last;
    }
  else
    for (last = first; NEXT_INSN (last); last = NEXT_INSN (last))
      continue;

  after_after = NEXT_INSN (after);

  SET_NEXT_INSN (after) = first;
  SET_PREV_INSN (first) = after;
  SET_NEXT_INSN (last) = after_after;
  if (after_after)
    SET_PREV_INSN (after_after) = last;

  if (after == get_last_insn ())
    set_last_insn (last);

  return last;
}


// Source: emit-rtl.c
// Lines 4615-4655
