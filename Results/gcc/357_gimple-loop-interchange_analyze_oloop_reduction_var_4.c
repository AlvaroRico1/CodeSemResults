loop_cand::analyze_oloop_reduction_var (loop_cand *iloop, tree var)
{
  gphi *phi = as_a <gphi *> (SSA_NAME_DEF_STMT (var));
  gphi *lcssa_phi = NULL, *use_phi;
  tree init = PHI_ARG_DEF_FROM_EDGE (phi, loop_preheader_edge (m_loop));
  tree next = PHI_ARG_DEF_FROM_EDGE (phi, loop_latch_edge (m_loop));
  reduction_p re;
  gimple *stmt, *next_def;
  use_operand_p use_p;
  imm_use_iterator iterator;

  if (TREE_CODE (next) != SSA_NAME)
    return false;

  next_def = SSA_NAME_DEF_STMT (next);
  basic_block bb = gimple_bb (next_def);
  if (!bb || !flow_bb_inside_loop_p (m_loop, bb))
    return false;

  /* Find inner loop's simple reduction that uses var as initializer.  */
  reduction_p inner_re = NULL;
  for (unsigned i = 0; iloop->m_reductions.iterate (i, &inner_re); ++i)
    if (inner_re->init == var || operand_equal_p (inner_re->init, var, 0))
      break;

  if (inner_re == NULL
      || inner_re->type != UNKNOWN_RTYPE
      || inner_re->producer != phi)
    return false;

  /* In case of double reduction, outer loop's reduction should be updated
     by inner loop's simple reduction.  */
  if (next_def != inner_re->lcssa_phi)
    return false;

  /* Outer loop's reduction should only be used to initialize inner loop's
     simple reduction.  */
  if (! single_imm_use (var, &use_p, &stmt)
      || stmt != inner_re->phi)
    return false;

  /* Check this reduction is correctly used outside of loop via lcssa phi.  */
  FOR_EACH_IMM_USE_FAST (use_p, iterator, next)
    {
      stmt = USE_STMT (use_p);
      if (is_gimple_debug (stmt))
	continue;

      /* Or else it's used in PHI itself.  */
      use_phi = dyn_cast <gphi *> (stmt);
      if (use_phi == phi)
	continue;

      if (lcssa_phi == NULL
	  && use_phi != NULL
	  && gimple_bb (stmt) == m_exit->dest
	  && PHI_ARG_DEF_FROM_EDGE (use_phi, m_exit) == next)
	lcssa_phi = use_phi;
      else
	return false;
    }
  if (!lcssa_phi)
    return false;

  re = XCNEW (struct reduction);
  re->var = var;
  re->init = init;
  re->next = next;
  re->phi = phi;
  re->lcssa_phi = lcssa_phi;
  re->type = DOUBLE_RTYPE;
  inner_re->type = DOUBLE_RTYPE;

  if (dump_file && (dump_flags & TDF_DETAILS))
    dump_reduction (re);

  m_reductions.safe_push (re);
  return true;
}


// Source: gimple-loop-interchange.cc
// Lines 599-677
