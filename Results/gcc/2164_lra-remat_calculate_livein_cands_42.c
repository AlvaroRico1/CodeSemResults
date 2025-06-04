calculate_livein_cands (void)
{
  basic_block bb;

  FOR_EACH_BB_FN (bb, cfun)
    {
      bitmap livein_regs = df_get_live_in (bb);
      bitmap livein_cands = &get_remat_bb_data (bb)->livein_cands;
      for (unsigned int i = 0; i < cands_num; i++)
	{
	  cand_t cand = all_cands[i];
	  lra_insn_recog_data_t id = lra_get_insn_recog_data (cand->insn);
	  struct lra_insn_reg *reg;

	  for (reg = id->regs; reg != NULL; reg = reg->next)
	    if (reg->type == OP_IN && ! bitmap_bit_p (livein_regs, reg->regno))
	      break;
	  if (reg == NULL)
	    bitmap_set_bit (livein_cands, i);
	}


// Source: lra-remat.c
// Lines 724-743
