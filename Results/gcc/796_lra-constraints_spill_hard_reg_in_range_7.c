spill_hard_reg_in_range (int regno, enum reg_class rclass, rtx_insn *from, rtx_insn *to)
{
  int i, hard_regno;
  int rclass_size;
  rtx_insn *insn;
  unsigned int uid;
  bitmap_iterator bi;
  HARD_REG_SET ignore;
  
  lra_assert (from != NULL && to != NULL);
  CLEAR_HARD_REG_SET (ignore);
  EXECUTE_IF_SET_IN_BITMAP (&lra_reg_info[regno].insn_bitmap, 0, uid, bi)
    {
      lra_insn_recog_data_t id = lra_insn_recog_data[uid];
      struct lra_static_insn_data *static_id = id->insn_static_data;
      struct lra_insn_reg *reg;
      
      for (reg = id->regs; reg != NULL; reg = reg->next)
	if (reg->regno < FIRST_PSEUDO_REGISTER)
	  SET_HARD_REG_BIT (ignore, reg->regno);
      for (reg = static_id->hard_regs; reg != NULL; reg = reg->next)
	SET_HARD_REG_BIT (ignore, reg->regno);
    }


// Source: lra-constraints.c
// Lines 5848-5870
