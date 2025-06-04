gimple_split_block (basic_block bb, void *stmt)
{
  gimple_stmt_iterator gsi;
  gimple_stmt_iterator gsi_tgt;
  gimple_seq list;
  basic_block new_bb;
  edge e;
  edge_iterator ei;

  new_bb = create_empty_bb (bb);

  /* Redirect the outgoing edges.  */
  new_bb->succs = bb->succs;
  bb->succs = NULL;
  FOR_EACH_EDGE (e, ei, new_bb->succs)
    e->src = new_bb;

  /* Get a stmt iterator pointing to the first stmt to move.  */
  if (!stmt || gimple_code ((gimple *) stmt) == GIMPLE_LABEL)
    gsi = gsi_after_labels (bb);
  else
    {
      gsi = gsi_for_stmt ((gimple *) stmt);
      gsi_next (&gsi);
    }
 
  /* Move everything from GSI to the new basic block.  */
  if (gsi_end_p (gsi))
    return new_bb;

  /* Split the statement list - avoid re-creating new containers as this
     brings ugly quadratic memory consumption in the inliner.
     (We are still quadratic since we need to update stmt BB pointers,
     sadly.)  */
  gsi_split_seq_before (&gsi, &list);
  set_bb_seq (new_bb, list);
  for (gsi_tgt = gsi_start (list);
       !gsi_end_p (gsi_tgt); gsi_next (&gsi_tgt))
    gimple_set_bb (gsi_stmt (gsi_tgt), new_bb);

  return new_bb;
}


// Source: tree-cfg.c
// Lines 6126-6167
