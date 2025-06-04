spill_at_interval (hsa_op_reg *reg, vec<hsa_op_reg*> *active)
{
  int cl = m_reg_class_for_type (reg->m_type);
  gcc_assert (!active[cl].is_empty ());
  hsa_op_reg *cand = active[cl][0];
  if (cand->m_lr_end > reg->m_lr_end)
    {
      reg->m_reg_class = cand->m_reg_class;
      reg->m_hard_num = cand->m_hard_num;
      active[cl].ordered_remove (0);
      unsigned place = active[cl].lower_bound (reg, cmp_end);
      active[cl].quick_insert (place, reg);
    }
  else
    cand = reg;

  gcc_assert (!cand->m_spill_sym);
  BrigType16_t type = cand->m_type;
  if (type == BRIG_TYPE_B1)
    type = BRIG_TYPE_U8;
  cand->m_reg_class = 0;
  cand->m_spill_sym = hsa_get_spill_symbol (type);
  cand->m_spill_sym->m_name_number = cand->m_order;
}


// Source: hsa-regalloc.c
// Lines 386-409
