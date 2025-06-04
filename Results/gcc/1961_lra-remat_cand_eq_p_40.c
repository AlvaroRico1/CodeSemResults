cand_eq_p (const void *cand1, const void *cand2)
{
  const_cand_t c1 = (const_cand_t) cand1;
  const_cand_t c2 = (const_cand_t) cand2;
  lra_insn_recog_data_t id1 = lra_get_insn_recog_data (c1->insn);
  lra_insn_recog_data_t id2 = lra_get_insn_recog_data (c2->insn);
  struct lra_static_insn_data *static_id1 = id1->insn_static_data;
  int nops = static_id1->n_operands;

  if (c1->regno != c2->regno
      || INSN_CODE (c1->insn) < 0
      || INSN_CODE (c1->insn) != INSN_CODE (c2->insn))
    return false;
  gcc_assert (c1->nop == c2->nop);
  for (int i = 0; i < nops; i++)
    if (i != c1->nop && static_id1->operand[i].type == OP_IN
	&& *id1->operand_loc[i] != *id2->operand_loc[i])
      return false;
  return true;
}


// Source: lra-remat.c
// Lines 194-213
