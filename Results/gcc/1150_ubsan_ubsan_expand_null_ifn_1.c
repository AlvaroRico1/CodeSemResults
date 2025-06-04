ubsan_expand_null_ifn (gimple_stmt_iterator *gsip)
{
  gimple_stmt_iterator gsi = *gsip;
  gimple *stmt = gsi_stmt (gsi);
  location_t loc = gimple_location (stmt);
  gcc_assert (gimple_call_num_args (stmt) == 3);
  tree ptr = gimple_call_arg (stmt, 0);
  tree ckind = gimple_call_arg (stmt, 1);
  tree align = gimple_call_arg (stmt, 2);
  tree check_align = NULL_TREE;
  bool check_null;

  basic_block cur_bb = gsi_bb (gsi);

  gimple *g;
  if (!integer_zerop (align))
    {
      unsigned int ptralign = get_pointer_alignment (ptr) / BITS_PER_UNIT;
      if (compare_tree_int (align, ptralign) == 1)
	{
	  check_align = make_ssa_name (pointer_sized_int_node);
	  g = gimple_build_assign (check_align, NOP_EXPR, ptr);
	  gimple_set_location (g, loc);
	  gsi_insert_before (&gsi, g, GSI_SAME_STMT);
	}
    }
  check_null = sanitize_flags_p (SANITIZE_NULL);

  if (check_align == NULL_TREE && !check_null)
    {
      gsi_remove (gsip, true);
      /* Unlink the UBSAN_NULLs vops before replacing it.  */
      unlink_stmt_vdef (stmt);
      return true;
    }

  /* Split the original block holding the pointer dereference.  */
  edge e = split_block (cur_bb, stmt);

  /* Get a hold on the 'condition block', the 'then block' and the
     'else block'.  */
  basic_block cond_bb = e->src;
  basic_block fallthru_bb = e->dest;
  basic_block then_bb = create_empty_bb (cond_bb);
  add_bb_to_loop (then_bb, cond_bb->loop_father);
  loops_state_set (LOOPS_NEED_FIXUP);

  /* Make an edge coming from the 'cond block' into the 'then block';
     this edge is unlikely taken, so set up the probability accordingly.  */
  e = make_edge (cond_bb, then_bb, EDGE_TRUE_VALUE);
  e->probability = profile_probability::very_unlikely ();
  then_bb->count = e->count ();

  /* Connect 'then block' with the 'else block'.  This is needed
     as the ubsan routines we call in the 'then block' are not noreturn.
     The 'then block' only has one outcoming edge.  */
  make_single_succ_edge (then_bb, fallthru_bb, EDGE_FALLTHRU);

  /* Set up the fallthrough basic block.  */
  e = find_edge (cond_bb, fallthru_bb);
  e->flags = EDGE_FALSE_VALUE;
  e->probability = profile_probability::very_likely ();

  /* Update dominance info for the newly created then_bb; note that
     fallthru_bb's dominance info has already been updated by
     split_block.  */
  if (dom_info_available_p (CDI_DOMINATORS))
    set_immediate_dominator (CDI_DOMINATORS, then_bb, cond_bb);

  /* Put the ubsan builtin call into the newly created BB.  */
  if (flag_sanitize_undefined_trap_on_error)
    g = gimple_build_call (builtin_decl_implicit (BUILT_IN_TRAP), 0);
  else
    {
      enum built_in_function bcode
	= (flag_sanitize_recover & ((check_align ? SANITIZE_ALIGNMENT : 0)
				    | (check_null ? SANITIZE_NULL : 0)))
	  ? BUILT_IN_UBSAN_HANDLE_TYPE_MISMATCH_V1
	  : BUILT_IN_UBSAN_HANDLE_TYPE_MISMATCH_V1_ABORT;
      tree fn = builtin_decl_implicit (bcode);
      int align_log = tree_log2 (align);
      tree data
	= ubsan_create_data ("__ubsan_null_data", 1, &loc,
			     ubsan_type_descriptor (TREE_TYPE (ckind),
						    UBSAN_PRINT_POINTER),
			     NULL_TREE,
			     build_int_cst (unsigned_char_type_node,
					    MAX (align_log, 0)),
			     fold_convert (unsigned_char_type_node, ckind),
			     NULL_TREE);
      data = build_fold_addr_expr_loc (loc, data);
      g = gimple_build_call (fn, 2, data,
			     check_align ? check_align
			     : build_zero_cst (pointer_sized_int_node));
    }
  gimple_stmt_iterator gsi2 = gsi_start_bb (then_bb);
  gimple_set_location (g, loc);
  gsi_insert_after (&gsi2, g, GSI_NEW_STMT);

  /* Unlink the UBSAN_NULLs vops before replacing it.  */
  unlink_stmt_vdef (stmt);

  if (check_null)
    {
      g = gimple_build_cond (EQ_EXPR, ptr, build_int_cst (TREE_TYPE (ptr), 0),
			     NULL_TREE, NULL_TREE);
      gimple_set_location (g, loc);

      /* Replace the UBSAN_NULL with a GIMPLE_COND stmt.  */
      gsi_replace (&gsi, g, false);
      stmt = g;
    }

  if (check_align)
    {
      if (check_null)
	{
	  /* Split the block with the condition again.  */
	  e = split_block (cond_bb, stmt);
	  basic_block cond1_bb = e->src;
	  basic_block cond2_bb = e->dest;

	  /* Make an edge coming from the 'cond1 block' into the 'then block';
	     this edge is unlikely taken, so set up the probability
	     accordingly.  */
	  e = make_edge (cond1_bb, then_bb, EDGE_TRUE_VALUE);
	  e->probability = profile_probability::very_unlikely ();

	  /* Set up the fallthrough basic block.  */
	  e = find_edge (cond1_bb, cond2_bb);
	  e->flags = EDGE_FALSE_VALUE;
	  e->probability = profile_probability::very_likely ();

	  /* Update dominance info.  */
	  if (dom_info_available_p (CDI_DOMINATORS))
	    {
	      set_immediate_dominator (CDI_DOMINATORS, fallthru_bb, cond1_bb);
	      set_immediate_dominator (CDI_DOMINATORS, then_bb, cond1_bb);
	    }

	  gsi2 = gsi_start_bb (cond2_bb);
	}

      tree mask = build_int_cst (pointer_sized_int_node,
				 tree_to_uhwi (align) - 1);
      g = gimple_build_assign (make_ssa_name (pointer_sized_int_node),
			       BIT_AND_EXPR, check_align, mask);
      gimple_set_location (g, loc);
      if (check_null)
	gsi_insert_after (&gsi2, g, GSI_NEW_STMT);
      else
	gsi_insert_before (&gsi, g, GSI_SAME_STMT);

      g = gimple_build_cond (NE_EXPR, gimple_assign_lhs (g),
			     build_int_cst (pointer_sized_int_node, 0),
			     NULL_TREE, NULL_TREE);
      gimple_set_location (g, loc);
      if (check_null)
	gsi_insert_after (&gsi2, g, GSI_NEW_STMT);
      else
	/* Replace the UBSAN_NULL with a GIMPLE_COND stmt.  */
	gsi_replace (&gsi, g, false);
    }
  return false;
}


// Source: ubsan.c
// Lines 757-921
