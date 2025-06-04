gsi_replace_with_seq_vops (gimple_stmt_iterator *si_p, gimple_seq stmts)
{
  gimple *stmt = gsi_stmt (*si_p);

  if (gimple_has_location (stmt))
    annotate_all_with_location (stmts, gimple_location (stmt));

  /* First iterate over the replacement statements backward, assigning
     virtual operands to their defining statements.  */
  gimple *laststore = NULL;
  for (gimple_stmt_iterator i = gsi_last (stmts);
       !gsi_end_p (i); gsi_prev (&i))
    {
      gimple *new_stmt = gsi_stmt (i);
      if ((gimple_assign_single_p (new_stmt)
	   && !is_gimple_reg (gimple_assign_lhs (new_stmt)))
	  || (is_gimple_call (new_stmt)
	      && (gimple_call_flags (new_stmt)
		  & (ECF_NOVOPS | ECF_PURE | ECF_CONST | ECF_NORETURN)) == 0))
	{
	  tree vdef;
	  if (!laststore)
	    vdef = gimple_vdef (stmt);
	  else
	    vdef = make_ssa_name (gimple_vop (cfun), new_stmt);
	  gimple_set_vdef (new_stmt, vdef);
	  if (vdef && TREE_CODE (vdef) == SSA_NAME)
	    SSA_NAME_DEF_STMT (vdef) = new_stmt;
	  laststore = new_stmt;
	}
    }


// Source: gimple-fold.c
// Lines 488-518
