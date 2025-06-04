update_ebb_live_info (rtx_insn *head, rtx_insn *tail)
{
  unsigned int j;
  int i, regno;
  bool live_p;
  rtx_insn *prev_insn;
  rtx set;
  bool remove_p;
  basic_block last_bb, prev_bb, curr_bb;
  bitmap_iterator bi;
  struct lra_insn_reg *reg;
  edge e;
  edge_iterator ei;

  last_bb = BLOCK_FOR_INSN (tail);
  prev_bb = NULL;
  for (curr_insn = tail;
       curr_insn != PREV_INSN (head);
       curr_insn = prev_insn)
    {
      prev_insn = PREV_INSN (curr_insn);
      /* We need to process empty blocks too.  They contain
	 NOTE_INSN_BASIC_BLOCK referring for the basic block.  */
      if (NOTE_P (curr_insn) && NOTE_KIND (curr_insn) != NOTE_INSN_BASIC_BLOCK)
	continue;
      curr_bb = BLOCK_FOR_INSN (curr_insn);
      if (curr_bb != prev_bb)
	{
	  if (prev_bb != NULL)
	    {
	      /* Update df_get_live_in (prev_bb):  */
	      EXECUTE_IF_SET_IN_BITMAP (&check_only_regs, 0, j, bi)
		if (bitmap_bit_p (&live_regs, j))
		  bitmap_set_bit (df_get_live_in (prev_bb), j);
		else
		  bitmap_clear_bit (df_get_live_in (prev_bb), j);
	    }
	  if (curr_bb != last_bb)
	    {
	      /* Update df_get_live_out (curr_bb):  */
	      EXECUTE_IF_SET_IN_BITMAP (&check_only_regs, 0, j, bi)
		{
		  live_p = bitmap_bit_p (&live_regs, j);
		  if (! live_p)
		    FOR_EACH_EDGE (e, ei, curr_bb->succs)
		      if (bitmap_bit_p (df_get_live_in (e->dest), j))
			{
			  live_p = true;
			  break;
			}
		  if (live_p)
		    bitmap_set_bit (df_get_live_out (curr_bb), j);
		  else
		    bitmap_clear_bit (df_get_live_out (curr_bb), j);
		}
	    }
	  prev_bb = curr_bb;
	  bitmap_and (&live_regs, &check_only_regs, df_get_live_out (curr_bb));
	}
      if (! NONDEBUG_INSN_P (curr_insn))
	continue;
      curr_id = lra_get_insn_recog_data (curr_insn);
      curr_static_id = curr_id->insn_static_data;
      remove_p = false;
      if ((set = single_set (curr_insn)) != NULL_RTX
	  && REG_P (SET_DEST (set))
	  && (regno = REGNO (SET_DEST (set))) >= FIRST_PSEUDO_REGISTER
	  && SET_DEST (set) != pic_offset_table_rtx
	  && bitmap_bit_p (&check_only_regs, regno)
	  && ! bitmap_bit_p (&live_regs, regno))
	remove_p = true;
      /* See which defined values die here.  */
      for (reg = curr_id->regs; reg != NULL; reg = reg->next)
	if (reg->type == OP_OUT && ! reg->subreg_p)
	  bitmap_clear_bit (&live_regs, reg->regno);
      for (reg = curr_static_id->hard_regs; reg != NULL; reg = reg->next)
	if (reg->type == OP_OUT && ! reg->subreg_p)
	  bitmap_clear_bit (&live_regs, reg->regno);
      if (curr_id->arg_hard_regs != NULL)
	/* Make clobbered argument hard registers die.  */
	for (i = 0; (regno = curr_id->arg_hard_regs[i]) >= 0; i++)
	  if (regno >= FIRST_PSEUDO_REGISTER)
	    bitmap_clear_bit (&live_regs, regno - FIRST_PSEUDO_REGISTER);
      /* Mark each used value as live.  */
      for (reg = curr_id->regs; reg != NULL; reg = reg->next)
	if (reg->type != OP_OUT
	    && bitmap_bit_p (&check_only_regs, reg->regno))
	  bitmap_set_bit (&live_regs, reg->regno);
      for (reg = curr_static_id->hard_regs; reg != NULL; reg = reg->next)
	if (reg->type != OP_OUT
	    && bitmap_bit_p (&check_only_regs, reg->regno))
	  bitmap_set_bit (&live_regs, reg->regno);
      if (curr_id->arg_hard_regs != NULL)
	/* Make used argument hard registers live.  */
	for (i = 0; (regno = curr_id->arg_hard_regs[i]) >= 0; i++)
	  if (regno < FIRST_PSEUDO_REGISTER
	      && bitmap_bit_p (&check_only_regs, regno))
	    bitmap_set_bit (&live_regs, regno);
      /* It is quite important to remove dead move insns because it
	 means removing dead store.  We don't need to process them for
	 constraints.  */
      if (remove_p)
	{
	  if (lra_dump_file != NULL)
	    {
	      fprintf (lra_dump_file, "	    Removing dead insn:\n ");
	      dump_insn_slim (lra_dump_file, curr_insn);
	    }
	  lra_set_insn_deleted (curr_insn);
	}
    }
}


// Source: lra-constraints.c
// Lines 6073-6184
