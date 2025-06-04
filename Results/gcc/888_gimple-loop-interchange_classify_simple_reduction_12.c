loop_cand::classify_simple_reduction (reduction_p re)
{
  gimple *producer, *consumer;

  /* Check init variable of reduction and how it is initialized.  */
  if (TREE_CODE (re->init) == SSA_NAME)
    {
      producer = SSA_NAME_DEF_STMT (re->init);
      re->producer = producer;
      basic_block bb = gimple_bb (producer);
      if (!bb || bb->loop_father != m_outer)
	return;

      if (!gimple_assign_load_p (producer))
	return;

      re->init_ref = gimple_assign_rhs1 (producer);
    }
  else if (CONSTANT_CLASS_P (re->init))
    m_const_init_reduc++;
  else
    return;

  /* Check how reduction variable is used.  */
  consumer = single_use_in_loop (PHI_RESULT (re->lcssa_phi), m_outer);
  if (!consumer
      || !gimple_store_p (consumer))
    return;

  re->fini_ref = gimple_get_lhs (consumer);
  re->consumer = consumer;

  /* Simple reduction with constant initializer.  */
  if (!re->init_ref)
    {
      gcc_assert (CONSTANT_CLASS_P (re->init));
      re->init_ref = unshare_expr (re->fini_ref);
    }

  /* Require memory references in producer and consumer are the same so
     that we can undo reduction during interchange.  */
  if (re->init_ref && !operand_equal_p (re->init_ref, re->fini_ref, 0))
    return;

  re->type = SIMPLE_RTYPE;
}


// Source: gimple-loop-interchange.cc
// Lines 412-457
