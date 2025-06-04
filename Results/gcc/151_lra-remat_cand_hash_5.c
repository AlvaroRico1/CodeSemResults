cand_hash (const void *cand)
{
  const_cand_t c = (const_cand_t) cand;
  lra_insn_recog_data_t id = lra_get_insn_recog_data (c->insn);
  struct lra_static_insn_data *static_id = id->insn_static_data;
  int nops = static_id->n_operands;
  hashval_t hash = 0;

  for (int i = 0; i < nops; i++)
    if (i == c->nop)
      hash = iterative_hash_object (c->regno, hash);
    else if (static_id->operand[i].type == OP_IN)
      hash = iterative_hash_object (*id->operand_loc[i], hash);
  return hash;
}


// Source: lra-remat.c
// Lines 174-188
