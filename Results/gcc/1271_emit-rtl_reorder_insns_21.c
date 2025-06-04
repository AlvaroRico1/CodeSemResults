reorder_insns (rtx_insn *from, rtx_insn *to, rtx_insn *after)
{
  rtx_insn *prev = PREV_INSN (from);
  basic_block bb, bb2;

  reorder_insns_nobb (from, to, after);

  if (!BARRIER_P (after)
      && (bb = BLOCK_FOR_INSN (after)))
    {
      rtx_insn *x;
      df_set_bb_dirty (bb);

      if (!BARRIER_P (from)
	  && (bb2 = BLOCK_FOR_INSN (from)))
	{
	  if (BB_END (bb2) == to)
	    BB_END (bb2) = prev;
	  df_set_bb_dirty (bb2);
	}

      if (BB_END (bb) == after)
	BB_END (bb) = to;

      for (x = from; x != NEXT_INSN (to); x = NEXT_INSN (x))
	if (!BARRIER_P (x))
	  df_insn_change_bb (x, bb);
    }


// Source: emit-rtl.c
// Lines 4445-4472
