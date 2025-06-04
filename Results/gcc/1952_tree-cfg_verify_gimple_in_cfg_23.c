verify_gimple_in_cfg (struct function *fn, bool verify_nothrow)
{
  basic_block bb;
  bool err = false;

  timevar_push (TV_TREE_STMT_VERIFY);
  hash_set<void *> visited;
  hash_set<gimple *> visited_throwing_stmts;

  /* Collect all BLOCKs referenced by the BLOCK tree of FN.  */
  hash_set<tree> blocks;
  if (DECL_INITIAL (fn->decl))
    {
      blocks.add (DECL_INITIAL (fn->decl));
      collect_subblocks (&blocks, DECL_INITIAL (fn->decl));
    }

  FOR_EACH_BB_FN (bb, fn)
    {
      gimple_stmt_iterator gsi;
      edge_iterator ei;
      edge e;

      for (gphi_iterator gpi = gsi_start_phis (bb);
	   !gsi_end_p (gpi);
	   gsi_next (&gpi))
	{
	  gphi *phi = gpi.phi ();
	  bool err2 = false;
	  unsigned i;

	  if (gimple_bb (phi) != bb)
	    {
	      error ("gimple_bb (phi) is set to a wrong basic block");
	      err2 = true;
	    }

	  err2 |= verify_gimple_phi (phi);

	  /* Only PHI arguments have locations.  */
	  if (gimple_location (phi) != UNKNOWN_LOCATION)
	    {
	      error ("PHI node with location");
	      err2 = true;
	    }

	  for (i = 0; i < gimple_phi_num_args (phi); i++)
	    {
	      tree arg = gimple_phi_arg_def (phi, i);
	      tree addr = walk_tree (&arg, verify_node_sharing_1,
				     &visited, NULL);
	      if (addr)
		{
		  error ("incorrect sharing of tree nodes");
		  debug_generic_expr (addr);
		  err2 |= true;
		}
	      location_t loc = gimple_phi_arg_location (phi, i);
	      if (virtual_operand_p (gimple_phi_result (phi))
		  && loc != UNKNOWN_LOCATION)
		{
		  error ("virtual PHI with argument locations");
		  err2 = true;
		}
	      addr = walk_tree (&arg, verify_expr_location_1, &blocks, NULL);
	      if (addr)
		{
		  debug_generic_expr (addr);
		  err2 = true;
		}
	      err2 |= verify_location (&blocks, loc);
	    }

	  if (err2)
	    debug_gimple_stmt (phi);
	  err |= err2;
	}

      for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi))
	{
	  gimple *stmt = gsi_stmt (gsi);
	  bool err2 = false;
	  struct walk_stmt_info wi;
	  tree addr;
	  int lp_nr;

	  if (gimple_bb (stmt) != bb)
	    {
	      error ("gimple_bb (stmt) is set to a wrong basic block");
	      err2 = true;
	    }

	  err2 |= verify_gimple_stmt (stmt);
	  err2 |= verify_location (&blocks, gimple_location (stmt));

	  memset (&wi, 0, sizeof (wi));
	  wi.info = (void *) &visited;
	  addr = walk_gimple_op (stmt, verify_node_sharing, &wi);
	  if (addr)
	    {
	      error ("incorrect sharing of tree nodes");
	      debug_generic_expr (addr);
	      err2 |= true;
	    }

	  memset (&wi, 0, sizeof (wi));
	  wi.info = (void *) &blocks;
	  addr = walk_gimple_op (stmt, verify_expr_location, &wi);
	  if (addr)
	    {
	      debug_generic_expr (addr);
	      err2 |= true;
	    }

	  /* If the statement is marked as part of an EH region, then it is
	     expected that the statement could throw.  Verify that when we
	     have optimizations that simplify statements such that we prove
	     that they cannot throw, that we update other data structures
	     to match.  */
	  lp_nr = lookup_stmt_eh_lp (stmt);
	  if (lp_nr != 0)
	    visited_throwing_stmts.add (stmt);
	  if (lp_nr > 0)
	    {
	      if (!stmt_could_throw_p (cfun, stmt))
		{
		  if (verify_nothrow)
		    {
		      error ("statement marked for throw, but doesn%'t");
		      err2 |= true;
		    }
		}
	      else if (!gsi_one_before_end_p (gsi))
		{
		  error ("statement marked for throw in middle of block");
		  err2 |= true;
		}
	    }

	  if (err2)
	    debug_gimple_stmt (stmt);
	  err |= err2;
	}


// Source: tree-cfg.c
// Lines 5346-5488
