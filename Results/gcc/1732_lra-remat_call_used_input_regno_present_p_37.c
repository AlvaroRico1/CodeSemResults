call_used_input_regno_present_p (const function_abi &abi, rtx_insn *insn)
{
  int iter;
  lra_insn_recog_data_t id = lra_get_insn_recog_data (insn);
  struct lra_static_insn_data *static_id = id->insn_static_data;
  struct lra_insn_reg *reg;

  for (iter = 0; iter < 2; iter++)
    for (reg = (iter == 0 ? id->regs : static_id->hard_regs);
	 reg != NULL;
	 reg = reg->next)
      if (reg->type == OP_IN
	  && reg->regno < FIRST_PSEUDO_REGISTER
	  && abi.clobbers_reg_p (reg->biggest_mode, reg->regno))
	return true;
  return false;
}


// Source: lra-remat.c
// Lines 704-720
