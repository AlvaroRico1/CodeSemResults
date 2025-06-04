hsa_append_insn_after (hsa_insn_basic *new_insn, hsa_insn_basic *old_insn)
{
  hsa_bb *hbb = hsa_bb_for_bb (old_insn->m_bb);

  if (hbb->m_last_insn == old_insn)
    hbb->m_last_insn = new_insn;
  new_insn->m_prev = old_insn;
  new_insn->m_next = old_insn->m_next;
  if (old_insn->m_next)
    old_insn->m_next->m_prev = new_insn;
  old_insn->m_next = new_insn;
}


// Source: hsa-gen.c
// Lines 1811-1822
