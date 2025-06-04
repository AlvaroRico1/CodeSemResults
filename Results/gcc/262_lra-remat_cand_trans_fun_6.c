cand_trans_fun (int bb_index, bitmap bb_in, bitmap bb_out)
{
  remat_bb_data_t bb_info;
  bitmap bb_livein, bb_changed_regs, bb_dead_regs;
  unsigned int cid;
  bitmap_iterator bi;

  bb_info = get_remat_bb_data_by_index (bb_index);
  bb_livein = &bb_info->livein_cands;
  bb_changed_regs = &bb_info->changed_regs;
  bb_dead_regs = &bb_info->dead_regs;
  /* Calculate killed avin cands -- cands whose regs are changed or
     becoming dead in the BB.  We calculate it here as we hope that
     repeated calculations are compensated by smaller size of BB_IN in
     comparison with all candidates number.  */
  bitmap_clear (&temp_bitmap);
  EXECUTE_IF_SET_IN_BITMAP (bb_in, 0, cid, bi)
    {
      cand_t cand = all_cands[cid];
      lra_insn_recog_data_t id = lra_get_insn_recog_data (cand->insn);
      struct lra_insn_reg *reg;

      if (! bitmap_bit_p (bb_livein, cid))
	{
	  bitmap_set_bit (&temp_bitmap, cid);
	  continue;
	}
      for (reg = id->regs; reg != NULL; reg = reg->next)
	/* Ignore all outputs which are not the regno for
	   rematerialization.  */
	if (reg->type == OP_OUT && reg->regno != cand->regno)
	  continue;
	else if (bitmap_bit_p (bb_changed_regs, reg->regno)
		 || bitmap_bit_p (bb_dead_regs, reg->regno))
	  {
	    bitmap_set_bit (&temp_bitmap, cid);
	    break;
	  }
      /* Check regno for rematerialization.  */
      if (bitmap_bit_p (bb_changed_regs, cand->regno)
	  || bitmap_bit_p (bb_dead_regs, cand->regno))
	bitmap_set_bit (&temp_bitmap, cid);
    }


// Source: lra-remat.c
// Lines 842-884
