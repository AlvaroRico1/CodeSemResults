find_bb_boundaries (basic_block bb)
{
  basic_block orig_bb = bb;
  rtx_insn *insn = BB_HEAD (bb);
  rtx_insn *end = BB_END (bb), *x;
  rtx_jump_table_data *table;
  rtx_insn *flow_transfer_insn = NULL;
  rtx_insn *debug_insn = NULL;
  edge fallthru = NULL;
  bool skip_purge;

  if (insn == end)
    return;

  if (DEBUG_INSN_P (insn) || DEBUG_INSN_P (end))
    {
      /* Check whether, without debug insns, the insn==end test above
	 would have caused us to return immediately, and behave the
	 same way even with debug insns.  If we don't do this, debug
	 insns could cause us to purge dead edges at different times,
	 which could in turn change the cfg and affect codegen
	 decisions in subtle but undesirable ways.  */
      while (insn != end && DEBUG_INSN_P (insn))
	insn = NEXT_INSN (insn);
      rtx_insn *e = end;
      while (insn != e && DEBUG_INSN_P (e))
	e = PREV_INSN (e);
      if (insn == e)
	{
	  /* If there are debug insns after a single insn that is a
	     control flow insn in the block, we'd have left right
	     away, but we should clean up the debug insns after the
	     control flow insn, because they can't remain in the same
	     block.  So, do the debug insn cleaning up, but then bail
	     out without purging dead edges as we would if the debug
	     insns hadn't been there.  */
	  if (e != end && !DEBUG_INSN_P (e) && control_flow_insn_p (e))
	    {
	      skip_purge = true;
	      flow_transfer_insn = e;
	      goto clean_up_debug_after_control_flow;
	    }
	  return;
	}
    }


// Source: cfgbuild.c
// Lines 438-482
