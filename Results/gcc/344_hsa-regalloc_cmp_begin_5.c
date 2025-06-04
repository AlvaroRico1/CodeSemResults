cmp_begin (const void *a, const void *b)
{
  const hsa_op_reg * const *rega = (const hsa_op_reg * const *)a;
  const hsa_op_reg * const *regb = (const hsa_op_reg * const *)b;
  int ret;
  if (rega == regb)
    return 0;
  ret = (*rega)->m_lr_begin - (*regb)->m_lr_begin;
  if (ret)
    return ret;
  return ((*rega)->m_order - (*regb)->m_order);
}


// Source: hsa-regalloc.c
// Lines 331-342
