gimple_verify_flow_info (void)
{
  int err = 0;
  basic_block bb;
  gimple_stmt_iterator gsi;
  gimple *stmt;
  edge e;
  edge_iterator ei;

  if (ENTRY_BLOCK_PTR_FOR_FN (cfun)->il.gimple.seq
      || ENTRY_BLOCK_PTR_FOR_FN (cfun)->il.gimple.phi_nodes)
    {
      error ("ENTRY_BLOCK has IL associated with it");
      err = 1;
    }

  if (EXIT_BLOCK_PTR_FOR_FN (cfun)->il.gimple.seq
      || EXIT_BLOCK_PTR_FOR_FN (cfun)->il.gimple.phi_nodes)
    {
      error ("EXIT_BLOCK has IL associated with it");
      err = 1;
    }

  FOR_EACH_EDGE (e, ei, EXIT_BLOCK_PTR_FOR_FN (cfun)->preds)
    if (e->flags & EDGE_FALLTHRU)
      {
	error ("fallthru to exit from bb %d", e->src->index);
	err = 1;
      }

  FOR_EACH_BB_FN (bb, cfun)
    {
      bool found_ctrl_stmt = false;

      stmt = NULL;

      /* Skip labels on the start of basic block.  */
      for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi))
	{
	  tree label;
	  gimple *prev_stmt = stmt;

	  stmt = gsi_stmt (gsi);

	  if (gimple_code (stmt) != GIMPLE_LABEL)
	    break;

	  label = gimple_label_label (as_a <glabel *> (stmt));
	  if (prev_stmt && DECL_NONLOCAL (label))
	    {
	      error ("nonlocal label ");
	      print_generic_expr (stderr, label);
	      fprintf (stderr, " is not first in a sequence of labels in bb %d",
		       bb->index);
	      err = 1;
	    }

	  if (prev_stmt && EH_LANDING_PAD_NR (label) != 0)
	    {
	      error ("EH landing pad label ");
	      print_generic_expr (stderr, label);
	      fprintf (stderr, " is not first in a sequence of labels in bb %d",
		       bb->index);
	      err = 1;
	    }

	  if (label_to_block (cfun, label) != bb)
	    {
	      error ("label ");
	      print_generic_expr (stderr, label);
	      fprintf (stderr, " to block does not match in bb %d",
		       bb->index);
	      err = 1;
	    }

	  if (decl_function_context (label) != current_function_decl)
	    {
	      error ("label ");
	      print_generic_expr (stderr, label);
	      fprintf (stderr, " has incorrect context in bb %d",
		       bb->index);
	      err = 1;
	    }
	}


// Source: tree-cfg.c
// Lines 5512-5595
