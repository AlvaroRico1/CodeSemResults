hsa_insert_insn_before (hsa_insn_basic *new_insn, hsa_insn_basic *old_insn)
{
  hsa_bb *hbb = hsa_bb_for_bb (old_insn->m_bb);

  if (hbb->m_first_insn == old_insn)
    hbb->m_first_insn = new_insn;
  new_insn->m_prev = old_insn->m_prev;
  new_insn->m_next = old_insn;
  if (old_insn->m_prev)
    old_insn->m_prev->m_next = new_insn;
  old_insn->m_prev = new_insn;
}


// Source: hsa-gen.c
// Lines 1794-1805
