insert_backedge_copies (void)
{
  basic_block bb;
  gphi_iterator gsi;

  mark_dfs_back_edges ();

  FOR_EACH_BB_FN (bb, cfun)
    {
      /* Mark block as possibly needing calculation of UIDs.  */
      bb->aux = &bb->aux;

      for (gsi = gsi_start_phis (bb); !gsi_end_p (gsi); gsi_next (&gsi))
	{
	  gphi *phi = gsi.phi ();
	  tree result = gimple_phi_result (phi);
	  size_t i;

	  if (virtual_operand_p (result))
	    continue;

	  for (i = 0; i < gimple_phi_num_args (phi); i++)
	    {
	      tree arg = gimple_phi_arg_def (phi, i);
	      edge e = gimple_phi_arg_edge (phi, i);
	      /* We are only interested in copies emitted on critical
                 backedges.  */
	      if (!(e->flags & EDGE_DFS_BACK)
		  || !EDGE_CRITICAL_P (e))
		continue;

	      /* If the argument is not an SSA_NAME, then we will need a
		 constant initialization.  If the argument is an SSA_NAME then
		 a copy statement may be needed.  First handle the case
		 where we cannot insert before the argument definition.  */
	      if (TREE_CODE (arg) != SSA_NAME
		  || (gimple_code (SSA_NAME_DEF_STMT (arg)) == GIMPLE_PHI
		      && trivially_conflicts_p (bb, result, arg)))
		{
		  tree name;
		  gassign *stmt;
		  gimple *last = NULL;
		  gimple_stmt_iterator gsi2;

		  gsi2 = gsi_last_bb (gimple_phi_arg_edge (phi, i)->src);
		  if (!gsi_end_p (gsi2))
		    last = gsi_stmt (gsi2);

		  /* In theory the only way we ought to get back to the
		     start of a loop should be with a COND_EXPR or GOTO_EXPR.
		     However, better safe than sorry.
		     If the block ends with a control statement or
		     something that might throw, then we have to
		     insert this assignment before the last
		     statement.  Else insert it after the last statement.  */
		  if (last && stmt_ends_bb_p (last))
		    {
		      /* If the last statement in the block is the definition
			 site of the PHI argument, then we can't insert
			 anything after it.  */
		      if (TREE_CODE (arg) == SSA_NAME
			  && SSA_NAME_DEF_STMT (arg) == last)
			continue;
		    }

		  /* Create a new instance of the underlying variable of the
		     PHI result.  */
		  name = copy_ssa_name (result);
		  stmt = gimple_build_assign (name,
					      gimple_phi_arg_def (phi, i));

		  /* copy location if present.  */
		  if (gimple_phi_arg_has_location (phi, i))
		    gimple_set_location (stmt,
					 gimple_phi_arg_location (phi, i));

		  /* Insert the new statement into the block and update
		     the PHI node.  */
		  if (last && stmt_ends_bb_p (last))
		    gsi_insert_before (&gsi2, stmt, GSI_NEW_STMT);
		  else
		    gsi_insert_after (&gsi2, stmt, GSI_NEW_STMT);
		  SET_PHI_ARG_DEF (phi, i, name);
		}
	      /* Insert a copy before the definition of the backedge value
		 and adjust all conflicting uses.  */
	      else if (trivially_conflicts_p (bb, result, arg))
		{
		  gimple *def = SSA_NAME_DEF_STMT (arg);
		  if (gimple_nop_p (def)
		      || gimple_code (def) == GIMPLE_PHI)
		    continue;
		  tree name = copy_ssa_name (result);
		  gimple *stmt = gimple_build_assign (name, result);
		  imm_use_iterator imm_iter;
		  gimple *use_stmt;
		  /* The following matches trivially_conflicts_p.  */
		  FOR_EACH_IMM_USE_STMT (use_stmt, imm_iter, result)
		    {
		      if (gimple_bb (use_stmt) != bb
			  || (gimple_code (use_stmt) != GIMPLE_PHI
			      && (maybe_renumber_stmts_bb (bb), true)
			      && gimple_uid (use_stmt) > gimple_uid (def)))
			{
			  use_operand_p use;
			  FOR_EACH_IMM_USE_ON_STMT (use, imm_iter)
			    SET_USE (use, name);
			}
		    }
		  gimple_stmt_iterator gsi = gsi_for_stmt (def);
		  gsi_insert_before (&gsi, stmt, GSI_SAME_STMT);
		}
	    }
	}

      /* Unmark this block again.  */
      bb->aux = NULL;
    }
}


// Source: tree-outof-ssa.c
// Lines 1166-1284
