get_live_on_other_edges (basic_block from, basic_block to, bitmap res)
{
  rtx_insn *last;
  struct lra_insn_reg *reg;
  edge e;
  edge_iterator ei;

  lra_assert (to != NULL);
  bitmap_clear (res);
  FOR_EACH_EDGE (e, ei, from->succs)
    if (e->dest != to)
      bitmap_ior_into (res, df_get_live_in (e->dest));
  last = get_last_insertion_point (from);
  if (! JUMP_P (last))
    return;
  curr_id = lra_get_insn_recog_data (last);
  for (reg = curr_id->regs; reg != NULL; reg = reg->next)
    if (reg->type != OP_IN)
      bitmap_set_bit (res, reg->regno);
}


// Source: lra-constraints.c
// Lines 6236-6255
