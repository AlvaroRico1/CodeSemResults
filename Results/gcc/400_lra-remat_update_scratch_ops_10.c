update_scratch_ops (rtx_insn *remat_insn)
{
  lra_insn_recog_data_t id = lra_get_insn_recog_data (remat_insn);
  struct lra_static_insn_data *static_id = id->insn_static_data;
  for (int i = 0; i < static_id->n_operands; i++)
    {
      rtx *loc = id->operand_loc[i];
      if (! REG_P (*loc))
	continue;
      int regno = REGNO (*loc);
      if (! lra_former_scratch_p (regno))
	continue;
      *loc = lra_create_new_reg (GET_MODE (*loc), *loc,
				 lra_get_allocno_class (regno),
				 "scratch pseudo copy");
      lra_register_new_scratch_op (remat_insn, i, id->icode);
    }


// Source: lra-remat.c
// Lines 1029-1045
