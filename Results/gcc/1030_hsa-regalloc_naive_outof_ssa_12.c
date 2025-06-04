naive_outof_ssa (void)
{
  basic_block bb;

  hsa_cfun->m_in_ssa = false;

  FOR_ALL_BB_FN (bb, cfun)
  {
    hsa_bb *hbb = hsa_bb_for_bb (bb);
    hsa_insn_phi *phi;

    /* naive_process_phi can call split_edge on an incoming edge which order if
       the incoming edges to the basic block and thus make it inconsistent with
       the ordering of PHI arguments, so we collect them in advance.  */
    auto_vec<edge, 8> predecessors;
    unsigned pred_count = EDGE_COUNT (bb->preds);
    for (unsigned i = 0; i < pred_count; i++)
      predecessors.safe_push (EDGE_PRED (bb, i));

    for (phi = hbb->m_first_phi;
	 phi;
	 phi = phi->m_next ? as_a <hsa_insn_phi *> (phi->m_next) : NULL)
      naive_process_phi (phi, predecessors);

    /* Zap PHI nodes, they will be deallocated when everything else will.  */
    hbb->m_first_phi = NULL;
    hbb->m_last_phi = NULL;
  }


// Source: hsa-regalloc.c
// Lines 82-109
