expand_omp_for_static_chunk (struct omp_region *region,
			     struct omp_for_data *fd, gimple *inner_stmt)
{
  tree n, s0, e0, e, t;
  tree trip_var, trip_init, trip_main, trip_back, nthreads, threadid;
  tree type, itype, vmain, vback, vextra;
  basic_block entry_bb, exit_bb, body_bb, seq_start_bb, iter_part_bb;
  basic_block trip_update_bb = NULL, cont_bb, collapse_bb = NULL, fin_bb;
  gimple_stmt_iterator gsi, gsip;
  edge se;
  bool broken_loop = region->cont == NULL;
  tree *counts = NULL;
  tree n1, n2, step;
  tree reductions = NULL_TREE;
  tree cond_var = NULL_TREE, condtemp = NULL_TREE;

  itype = type = TREE_TYPE (fd->loop.v);
  if (POINTER_TYPE_P (type))
    itype = signed_type_for (type);

  entry_bb = region->entry;
  se = split_block (entry_bb, last_stmt (entry_bb));
  entry_bb = se->src;
  iter_part_bb = se->dest;
  cont_bb = region->cont;
  gcc_assert (EDGE_COUNT (iter_part_bb->succs) == 2);
  fin_bb = BRANCH_EDGE (iter_part_bb)->dest;
  gcc_assert (broken_loop
	      || fin_bb == FALLTHRU_EDGE (cont_bb)->dest);
  seq_start_bb = split_edge (FALLTHRU_EDGE (iter_part_bb));
  body_bb = single_succ (seq_start_bb);
  if (!broken_loop)
    {
      gcc_assert (BRANCH_EDGE (cont_bb)->dest == body_bb
		  || single_succ (BRANCH_EDGE (cont_bb)->dest) == body_bb);
      gcc_assert (EDGE_COUNT (cont_bb->succs) == 2);
      trip_update_bb = split_edge (FALLTHRU_EDGE (cont_bb));
    }
  exit_bb = region->exit;

  /* Trip and adjustment setup goes in ENTRY_BB.  */
  gsi = gsi_last_nondebug_bb (entry_bb);
  gcc_assert (gimple_code (gsi_stmt (gsi)) == GIMPLE_OMP_FOR);
  gsip = gsi;
  gsi_prev (&gsip);

  if (fd->collapse > 1)
    {
      int first_zero_iter = -1, dummy = -1;
      basic_block l2_dom_bb = NULL, dummy_bb = NULL;

      counts = XALLOCAVEC (tree, fd->collapse);
      expand_omp_for_init_counts (fd, &gsi, entry_bb, counts,
				  fin_bb, first_zero_iter,
				  dummy_bb, dummy, l2_dom_bb);
      t = NULL_TREE;
    }
  else if (gimple_omp_for_combined_into_p (fd->for_stmt))
    t = integer_one_node;
  else
    t = fold_binary (fd->loop.cond_code, boolean_type_node,
		     fold_convert (type, fd->loop.n1),
		     fold_convert (type, fd->loop.n2));
  if (fd->collapse == 1
      && TYPE_UNSIGNED (type)
      && (t == NULL_TREE || !integer_onep (t)))
    {
      n1 = fold_convert (type, unshare_expr (fd->loop.n1));
      n1 = force_gimple_operand_gsi (&gsi, n1, true, NULL_TREE,
				     true, GSI_SAME_STMT);
      n2 = fold_convert (type, unshare_expr (fd->loop.n2));
      n2 = force_gimple_operand_gsi (&gsi, n2, true, NULL_TREE,
				     true, GSI_SAME_STMT);
      gcond *cond_stmt = gimple_build_cond (fd->loop.cond_code, n1, n2,
						 NULL_TREE, NULL_TREE);
      gsi_insert_before (&gsi, cond_stmt, GSI_SAME_STMT);
      if (walk_tree (gimple_cond_lhs_ptr (cond_stmt),
		     expand_omp_regimplify_p, NULL, NULL)
	  || walk_tree (gimple_cond_rhs_ptr (cond_stmt),
			expand_omp_regimplify_p, NULL, NULL))
	{
	  gsi = gsi_for_stmt (cond_stmt);
	  gimple_regimplify_operands (cond_stmt, &gsi);
	}
      se = split_block (entry_bb, cond_stmt);
      se->flags = EDGE_TRUE_VALUE;
      entry_bb = se->dest;
      se->probability = profile_probability::very_likely ();
      se = make_edge (se->src, fin_bb, EDGE_FALSE_VALUE);
      se->probability = profile_probability::very_unlikely ();
      if (gimple_in_ssa_p (cfun))
	{
	  int dest_idx = find_edge (iter_part_bb, fin_bb)->dest_idx;
	  for (gphi_iterator gpi = gsi_start_phis (fin_bb);
	       !gsi_end_p (gpi); gsi_next (&gpi))
	    {
	      gphi *phi = gpi.phi ();
	      add_phi_arg (phi, gimple_phi_arg_def (phi, dest_idx),
			   se, UNKNOWN_LOCATION);
	    }
	}
      gsi = gsi_last_bb (entry_bb);
    }

  if (fd->lastprivate_conditional)
    {
      tree clauses = gimple_omp_for_clauses (fd->for_stmt);
      tree c = omp_find_clause (clauses, OMP_CLAUSE__CONDTEMP_);
      if (fd->have_pointer_condtemp)
	condtemp = OMP_CLAUSE_DECL (c);
      c = omp_find_clause (OMP_CLAUSE_CHAIN (c), OMP_CLAUSE__CONDTEMP_);
      cond_var = OMP_CLAUSE_DECL (c);
    }
  if (fd->have_reductemp || fd->have_pointer_condtemp)
    {
      tree t1 = build_int_cst (long_integer_type_node, 0);
      tree t2 = build_int_cst (long_integer_type_node, 1);
      tree t3 = build_int_cstu (long_integer_type_node,
				(HOST_WIDE_INT_1U << 31) + 1);
      tree clauses = gimple_omp_for_clauses (fd->for_stmt);
      gimple_stmt_iterator gsi2 = gsi_none ();
      gimple *g = NULL;
      tree mem = null_pointer_node, memv = NULL_TREE;
      if (fd->have_reductemp)
	{
	  tree c = omp_find_clause (clauses, OMP_CLAUSE__REDUCTEMP_);
	  reductions = OMP_CLAUSE_DECL (c);
	  gcc_assert (TREE_CODE (reductions) == SSA_NAME);
	  g = SSA_NAME_DEF_STMT (reductions);
	  reductions = gimple_assign_rhs1 (g);
	  OMP_CLAUSE_DECL (c) = reductions;
	  gsi2 = gsi_for_stmt (g);
	}
      else
	{
	  if (gsi_end_p (gsip))
	    gsi2 = gsi_after_labels (region->entry);
	  else
	    gsi2 = gsip;
	  reductions = null_pointer_node;
	}
      if (fd->have_pointer_condtemp)
	{
	  tree type = TREE_TYPE (condtemp);
	  memv = create_tmp_var (type);
	  TREE_ADDRESSABLE (memv) = 1;
	  unsigned HOST_WIDE_INT sz
	    = tree_to_uhwi (TYPE_SIZE_UNIT (TREE_TYPE (type)));
	  sz *= fd->lastprivate_conditional;
	  expand_omp_build_assign (&gsi2, memv, build_int_cst (type, sz),
				   false);
	  mem = build_fold_addr_expr (memv);
	}
      tree t
	= build_call_expr (builtin_decl_explicit (BUILT_IN_GOMP_LOOP_START),
			   9, t1, t2, t2, t3, t1, null_pointer_node,
			   null_pointer_node, reductions, mem);
      force_gimple_operand_gsi (&gsi2, t, true, NULL_TREE,
				true, GSI_SAME_STMT);
      if (fd->have_pointer_condtemp)
	expand_omp_build_assign (&gsi2, condtemp, memv, false);
      if (fd->have_reductemp)
	{
	  gsi_remove (&gsi2, true);
	  release_ssa_name (gimple_assign_lhs (g));
	}
    }
  switch (gimple_omp_for_kind (fd->for_stmt))
    {
    case GF_OMP_FOR_KIND_FOR:
      nthreads = builtin_decl_explicit (BUILT_IN_OMP_GET_NUM_THREADS);
      threadid = builtin_decl_explicit (BUILT_IN_OMP_GET_THREAD_NUM);
      break;
    case GF_OMP_FOR_KIND_DISTRIBUTE:
      nthreads = builtin_decl_explicit (BUILT_IN_OMP_GET_NUM_TEAMS);
      threadid = builtin_decl_explicit (BUILT_IN_OMP_GET_TEAM_NUM);
      break;
    default:
      gcc_unreachable ();
    }
  nthreads = build_call_expr (nthreads, 0);
  nthreads = fold_convert (itype, nthreads);
  nthreads = force_gimple_operand_gsi (&gsi, nthreads, true, NULL_TREE,
				       true, GSI_SAME_STMT);
  threadid = build_call_expr (threadid, 0);
  threadid = fold_convert (itype, threadid);
  threadid = force_gimple_operand_gsi (&gsi, threadid, true, NULL_TREE,
				       true, GSI_SAME_STMT);

  n1 = fd->loop.n1;
  n2 = fd->loop.n2;
  step = fd->loop.step;
  if (gimple_omp_for_combined_into_p (fd->for_stmt))
    {
      tree innerc = omp_find_clause (gimple_omp_for_clauses (fd->for_stmt),
				     OMP_CLAUSE__LOOPTEMP_);
      gcc_assert (innerc);
      n1 = OMP_CLAUSE_DECL (innerc);
      innerc = omp_find_clause (OMP_CLAUSE_CHAIN (innerc),
				OMP_CLAUSE__LOOPTEMP_);
      gcc_assert (innerc);
      n2 = OMP_CLAUSE_DECL (innerc);
    }
  n1 = force_gimple_operand_gsi (&gsi, fold_convert (type, n1),
				 true, NULL_TREE, true, GSI_SAME_STMT);
  n2 = force_gimple_operand_gsi (&gsi, fold_convert (itype, n2),
				 true, NULL_TREE, true, GSI_SAME_STMT);
  step = force_gimple_operand_gsi (&gsi, fold_convert (itype, step),
				   true, NULL_TREE, true, GSI_SAME_STMT);
  tree chunk_size = fold_convert (itype, fd->chunk_size);
  chunk_size = omp_adjust_chunk_size (chunk_size, fd->simd_schedule);
  chunk_size
    = force_gimple_operand_gsi (&gsi, chunk_size, true, NULL_TREE, true,
				GSI_SAME_STMT);

  t = build_int_cst (itype, (fd->loop.cond_code == LT_EXPR ? -1 : 1));
  t = fold_build2 (PLUS_EXPR, itype, step, t);
  t = fold_build2 (PLUS_EXPR, itype, t, n2);
  t = fold_build2 (MINUS_EXPR, itype, t, fold_convert (itype, n1));
  if (TYPE_UNSIGNED (itype) && fd->loop.cond_code == GT_EXPR)
    t = fold_build2 (TRUNC_DIV_EXPR, itype,
		     fold_build1 (NEGATE_EXPR, itype, t),
		     fold_build1 (NEGATE_EXPR, itype, step));
  else
    t = fold_build2 (TRUNC_DIV_EXPR, itype, t, step);
  t = fold_convert (itype, t);
  n = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				true, GSI_SAME_STMT);

  trip_var = create_tmp_reg (itype, ".trip");
  if (gimple_in_ssa_p (cfun))
    {
      trip_init = make_ssa_name (trip_var);
      trip_main = make_ssa_name (trip_var);
      trip_back = make_ssa_name (trip_var);
    }
  else
    {
      trip_init = trip_var;
      trip_main = trip_var;
      trip_back = trip_var;
    }

  gassign *assign_stmt
    = gimple_build_assign (trip_init, build_int_cst (itype, 0));
  gsi_insert_before (&gsi, assign_stmt, GSI_SAME_STMT);

  t = fold_build2 (MULT_EXPR, itype, threadid, chunk_size);
  t = fold_build2 (MULT_EXPR, itype, t, step);
  if (POINTER_TYPE_P (type))
    t = fold_build_pointer_plus (n1, t);
  else
    t = fold_build2 (PLUS_EXPR, type, t, n1);
  vextra = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				     true, GSI_SAME_STMT);

  /* Remove the GIMPLE_OMP_FOR.  */
  gsi_remove (&gsi, true);

  gimple_stmt_iterator gsif = gsi;

  /* Iteration space partitioning goes in ITER_PART_BB.  */
  gsi = gsi_last_bb (iter_part_bb);

  t = fold_build2 (MULT_EXPR, itype, trip_main, nthreads);
  t = fold_build2 (PLUS_EXPR, itype, t, threadid);
  t = fold_build2 (MULT_EXPR, itype, t, chunk_size);
  s0 = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				 false, GSI_CONTINUE_LINKING);

  t = fold_build2 (PLUS_EXPR, itype, s0, chunk_size);
  t = fold_build2 (MIN_EXPR, itype, t, n);
  e0 = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				 false, GSI_CONTINUE_LINKING);

  t = build2 (LT_EXPR, boolean_type_node, s0, n);
  gsi_insert_after (&gsi, gimple_build_cond_empty (t), GSI_CONTINUE_LINKING);

  /* Setup code for sequential iteration goes in SEQ_START_BB.  */
  gsi = gsi_start_bb (seq_start_bb);

  tree startvar = fd->loop.v;
  tree endvar = NULL_TREE;

  if (gimple_omp_for_combined_p (fd->for_stmt))
    {
      tree clauses = gimple_code (inner_stmt) == GIMPLE_OMP_PARALLEL
		     ? gimple_omp_parallel_clauses (inner_stmt)
		     : gimple_omp_for_clauses (inner_stmt);
      tree innerc = omp_find_clause (clauses, OMP_CLAUSE__LOOPTEMP_);
      gcc_assert (innerc);
      startvar = OMP_CLAUSE_DECL (innerc);
      innerc = omp_find_clause (OMP_CLAUSE_CHAIN (innerc),
				OMP_CLAUSE__LOOPTEMP_);
      gcc_assert (innerc);
      endvar = OMP_CLAUSE_DECL (innerc);
      if (fd->collapse > 1 && TREE_CODE (fd->loop.n2) != INTEGER_CST
	  && gimple_omp_for_kind (fd->for_stmt) == GF_OMP_FOR_KIND_DISTRIBUTE)
	{
	  int i;
	  for (i = 1; i < fd->collapse; i++)
	    {
	      innerc = omp_find_clause (OMP_CLAUSE_CHAIN (innerc),
					OMP_CLAUSE__LOOPTEMP_);
	      gcc_assert (innerc);
	    }
	  innerc = omp_find_clause (OMP_CLAUSE_CHAIN (innerc),
				    OMP_CLAUSE__LOOPTEMP_);
	  if (innerc)
	    {
	      /* If needed (distribute parallel for with lastprivate),
		 propagate down the total number of iterations.  */
	      tree t = fold_convert (TREE_TYPE (OMP_CLAUSE_DECL (innerc)),
				     fd->loop.n2);
	      t = force_gimple_operand_gsi (&gsi, t, false, NULL_TREE, false,
					    GSI_CONTINUE_LINKING);
	      assign_stmt = gimple_build_assign (OMP_CLAUSE_DECL (innerc), t);
	      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
	    }
	}
    }

  t = fold_convert (itype, s0);
  t = fold_build2 (MULT_EXPR, itype, t, step);
  if (POINTER_TYPE_P (type))
    {
      t = fold_build_pointer_plus (n1, t);
      if (!POINTER_TYPE_P (TREE_TYPE (startvar))
	  && TYPE_PRECISION (TREE_TYPE (startvar)) > TYPE_PRECISION (type))
	t = fold_convert (signed_type_for (type), t);
    }
  else
    t = fold_build2 (PLUS_EXPR, type, t, n1);
  t = fold_convert (TREE_TYPE (startvar), t);
  t = force_gimple_operand_gsi (&gsi, t,
				DECL_P (startvar)
				&& TREE_ADDRESSABLE (startvar),
				NULL_TREE, false, GSI_CONTINUE_LINKING);
  assign_stmt = gimple_build_assign (startvar, t);
  gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
  if (cond_var)
    {
      tree itype = TREE_TYPE (cond_var);
      /* For lastprivate(conditional:) itervar, we need some iteration
	 counter that starts at unsigned non-zero and increases.
	 Prefer as few IVs as possible, so if we can use startvar
	 itself, use that, or startvar + constant (those would be
	 incremented with step), and as last resort use the s0 + 1
	 incremented by 1.  */
      if (POINTER_TYPE_P (type)
	  || TREE_CODE (n1) != INTEGER_CST
	  || fd->loop.cond_code != LT_EXPR)
	t = fold_build2 (PLUS_EXPR, itype, fold_convert (itype, s0),
			 build_int_cst (itype, 1));
      else if (tree_int_cst_sgn (n1) == 1)
	t = fold_convert (itype, t);
      else
	{
	  tree c = fold_convert (itype, n1);
	  c = fold_build2 (MINUS_EXPR, itype, build_int_cst (itype, 1), c);
	  t = fold_build2 (PLUS_EXPR, itype, fold_convert (itype, t), c);
	}
      t = force_gimple_operand_gsi (&gsi, t, false,
				    NULL_TREE, false, GSI_CONTINUE_LINKING);
      assign_stmt = gimple_build_assign (cond_var, t);
      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
    }

  t = fold_convert (itype, e0);
  t = fold_build2 (MULT_EXPR, itype, t, step);
  if (POINTER_TYPE_P (type))
    {
      t = fold_build_pointer_plus (n1, t);
      if (!POINTER_TYPE_P (TREE_TYPE (startvar))
	  && TYPE_PRECISION (TREE_TYPE (startvar)) > TYPE_PRECISION (type))
	t = fold_convert (signed_type_for (type), t);
    }
  else
    t = fold_build2 (PLUS_EXPR, type, t, n1);
  t = fold_convert (TREE_TYPE (startvar), t);
  e = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				false, GSI_CONTINUE_LINKING);
  if (endvar)
    {
      assign_stmt = gimple_build_assign (endvar, e);
      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
      if (useless_type_conversion_p (TREE_TYPE (fd->loop.v), TREE_TYPE (e)))
	assign_stmt = gimple_build_assign (fd->loop.v, e);
      else
	assign_stmt = gimple_build_assign (fd->loop.v, NOP_EXPR, e);
      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
    }
  /* Handle linear clause adjustments.  */
  tree itercnt = NULL_TREE, itercntbias = NULL_TREE;
  if (gimple_omp_for_kind (fd->for_stmt) == GF_OMP_FOR_KIND_FOR)
    for (tree c = gimple_omp_for_clauses (fd->for_stmt);
	 c; c = OMP_CLAUSE_CHAIN (c))
      if (OMP_CLAUSE_CODE (c) == OMP_CLAUSE_LINEAR
	  && !OMP_CLAUSE_LINEAR_NO_COPYIN (c))
	{
	  tree d = OMP_CLAUSE_DECL (c);
	  bool is_ref = omp_is_reference (d);
	  tree t = d, a, dest;
	  if (is_ref)
	    t = build_simple_mem_ref_loc (OMP_CLAUSE_LOCATION (c), t);
	  tree type = TREE_TYPE (t);
	  if (POINTER_TYPE_P (type))
	    type = sizetype;
	  dest = unshare_expr (t);
	  tree v = create_tmp_var (TREE_TYPE (t), NULL);
	  expand_omp_build_assign (&gsif, v, t);
	  if (itercnt == NULL_TREE)
	    {
	      if (gimple_omp_for_combined_into_p (fd->for_stmt))
		{
		  itercntbias
		    = fold_build2 (MINUS_EXPR, itype, fold_convert (itype, n1),
				   fold_convert (itype, fd->loop.n1));
		  itercntbias = fold_build2 (EXACT_DIV_EXPR, itype,
					     itercntbias, step);
		  itercntbias
		    = force_gimple_operand_gsi (&gsif, itercntbias, true,
						NULL_TREE, true,
						GSI_SAME_STMT);
		  itercnt = fold_build2 (PLUS_EXPR, itype, itercntbias, s0);
		  itercnt = force_gimple_operand_gsi (&gsi, itercnt, true,
						      NULL_TREE, false,
						      GSI_CONTINUE_LINKING);
		}
	      else
		itercnt = s0;
	    }
	  a = fold_build2 (MULT_EXPR, type,
			   fold_convert (type, itercnt),
			   fold_convert (type, OMP_CLAUSE_LINEAR_STEP (c)));
	  t = fold_build2 (type == TREE_TYPE (t) ? PLUS_EXPR
			   : POINTER_PLUS_EXPR, TREE_TYPE (t), v, a);
	  t = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
					false, GSI_CONTINUE_LINKING);
	  assign_stmt = gimple_build_assign (dest, t);
	  gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
	}
  if (fd->collapse > 1)
    expand_omp_for_init_vars (fd, &gsi, counts, inner_stmt, startvar);

  if (!broken_loop)
    {
      /* The code controlling the sequential loop goes in CONT_BB,
	 replacing the GIMPLE_OMP_CONTINUE.  */
      gsi = gsi_last_nondebug_bb (cont_bb);
      gomp_continue *cont_stmt = as_a <gomp_continue *> (gsi_stmt (gsi));
      vmain = gimple_omp_continue_control_use (cont_stmt);
      vback = gimple_omp_continue_control_def (cont_stmt);

      if (cond_var)
	{
	  tree itype = TREE_TYPE (cond_var);
	  tree t2;
	  if (POINTER_TYPE_P (type)
	      || TREE_CODE (n1) != INTEGER_CST
	      || fd->loop.cond_code != LT_EXPR)
	    t2 = build_int_cst (itype, 1);
	  else
	    t2 = fold_convert (itype, step);
	  t2 = fold_build2 (PLUS_EXPR, itype, cond_var, t2);
	  t2 = force_gimple_operand_gsi (&gsi, t2, false,
					 NULL_TREE, true, GSI_SAME_STMT);
	  assign_stmt = gimple_build_assign (cond_var, t2);
	  gsi_insert_before (&gsi, assign_stmt, GSI_SAME_STMT);
	}

      if (!gimple_omp_for_combined_p (fd->for_stmt))
	{
	  if (POINTER_TYPE_P (type))
	    t = fold_build_pointer_plus (vmain, step);
	  else
	    t = fold_build2 (PLUS_EXPR, type, vmain, step);
	  if (DECL_P (vback) && TREE_ADDRESSABLE (vback))
	    t = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
					  true, GSI_SAME_STMT);
	  assign_stmt = gimple_build_assign (vback, t);
	  gsi_insert_before (&gsi, assign_stmt, GSI_SAME_STMT);

	  if (tree_int_cst_equal (fd->chunk_size, integer_one_node))
	    t = build2 (EQ_EXPR, boolean_type_node,
			build_int_cst (itype, 0),
			build_int_cst (itype, 1));
	  else
	    t = build2 (fd->loop.cond_code, boolean_type_node,
			DECL_P (vback) && TREE_ADDRESSABLE (vback)
			? t : vback, e);
	  gsi_insert_before (&gsi, gimple_build_cond_empty (t), GSI_SAME_STMT);
	}

      /* Remove GIMPLE_OMP_CONTINUE.  */
      gsi_remove (&gsi, true);

      if (fd->collapse > 1 && !gimple_omp_for_combined_p (fd->for_stmt))
	collapse_bb = extract_omp_for_update_vars (fd, cont_bb, body_bb);

      /* Trip update code goes into TRIP_UPDATE_BB.  */
      gsi = gsi_start_bb (trip_update_bb);

      t = build_int_cst (itype, 1);
      t = build2 (PLUS_EXPR, itype, trip_main, t);
      assign_stmt = gimple_build_assign (trip_back, t);
      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
    }

  /* Replace the GIMPLE_OMP_RETURN with a barrier, or nothing.  */
  gsi = gsi_last_nondebug_bb (exit_bb);
  if (!gimple_omp_return_nowait_p (gsi_stmt (gsi)))
    {
      t = gimple_omp_return_lhs (gsi_stmt (gsi));
      if (fd->have_reductemp || fd->have_pointer_condtemp)
	{
	  tree fn;
	  if (t)
	    fn = builtin_decl_explicit (BUILT_IN_GOMP_LOOP_END_CANCEL);
	  else
	    fn = builtin_decl_explicit (BUILT_IN_GOMP_LOOP_END);
	  gcall *g = gimple_build_call (fn, 0);
	  if (t)
	    {
	      gimple_call_set_lhs (g, t);
	      if (fd->have_reductemp)
		gsi_insert_after (&gsi, gimple_build_assign (reductions,
							     NOP_EXPR, t),
				  GSI_SAME_STMT);
	    }
	  gsi_insert_after (&gsi, g, GSI_SAME_STMT);
	}
      else
	gsi_insert_after (&gsi, omp_build_barrier (t), GSI_SAME_STMT);
    }
  else if (fd->have_pointer_condtemp)
    {
      tree fn = builtin_decl_explicit (BUILT_IN_GOMP_LOOP_END_NOWAIT);
      gcall *g = gimple_build_call (fn, 0);
      gsi_insert_after (&gsi, g, GSI_SAME_STMT);
    }
  gsi_remove (&gsi, true);

  /* Connect the new blocks.  */
  find_edge (iter_part_bb, seq_start_bb)->flags = EDGE_TRUE_VALUE;
  find_edge (iter_part_bb, fin_bb)->flags = EDGE_FALSE_VALUE;

  if (!broken_loop)
    {
      se = find_edge (cont_bb, body_bb);
      if (se == NULL)
	{
	  se = BRANCH_EDGE (cont_bb);
	  gcc_assert (single_succ (se->dest) == body_bb);
	}
      if (gimple_omp_for_combined_p (fd->for_stmt))
	{
	  remove_edge (se);
	  se = NULL;
	}
      else if (fd->collapse > 1)
	{
	  remove_edge (se);
	  se = make_edge (cont_bb, collapse_bb, EDGE_TRUE_VALUE);
	}
      else
	se->flags = EDGE_TRUE_VALUE;
      find_edge (cont_bb, trip_update_bb)->flags
	= se ? EDGE_FALSE_VALUE : EDGE_FALLTHRU;

      redirect_edge_and_branch (single_succ_edge (trip_update_bb),
				iter_part_bb);
    }

  if (gimple_in_ssa_p (cfun))
    {
      gphi_iterator psi;
      gphi *phi;
      edge re, ene;
      edge_var_map *vm;
      size_t i;

      gcc_assert (fd->collapse == 1 && !broken_loop);

      /* When we redirect the edge from trip_update_bb to iter_part_bb, we
	 remove arguments of the phi nodes in fin_bb.  We need to create
	 appropriate phi nodes in iter_part_bb instead.  */
      se = find_edge (iter_part_bb, fin_bb);
      re = single_succ_edge (trip_update_bb);
      vec<edge_var_map> *head = redirect_edge_var_map_vector (re);
      ene = single_succ_edge (entry_bb);

      psi = gsi_start_phis (fin_bb);
      for (i = 0; !gsi_end_p (psi) && head->iterate (i, &vm);
	   gsi_next (&psi), ++i)
	{
	  gphi *nphi;
	  location_t locus;

	  phi = psi.phi ();
	  if (operand_equal_p (gimple_phi_arg_def (phi, 0),
			       redirect_edge_var_map_def (vm), 0))
	    continue;

	  t = gimple_phi_result (phi);
	  gcc_assert (t == redirect_edge_var_map_result (vm));

	  if (!single_pred_p (fin_bb))
	    t = copy_ssa_name (t, phi);

	  nphi = create_phi_node (t, iter_part_bb);

	  t = PHI_ARG_DEF_FROM_EDGE (phi, se);
	  locus = gimple_phi_arg_location_from_edge (phi, se);

	  /* A special case -- fd->loop.v is not yet computed in
	     iter_part_bb, we need to use vextra instead.  */
	  if (t == fd->loop.v)
	    t = vextra;
	  add_phi_arg (nphi, t, ene, locus);
	  locus = redirect_edge_var_map_location (vm);
	  tree back_arg = redirect_edge_var_map_def (vm);
	  add_phi_arg (nphi, back_arg, re, locus);
	  edge ce = find_edge (cont_bb, body_bb);
	  if (ce == NULL)
	    {
	      ce = BRANCH_EDGE (cont_bb);
	      gcc_assert (single_succ (ce->dest) == body_bb);
	      ce = single_succ_edge (ce->dest);
	    }
	  gphi *inner_loop_phi = find_phi_with_arg_on_edge (back_arg, ce);
	  gcc_assert (inner_loop_phi != NULL);
	  add_phi_arg (inner_loop_phi, gimple_phi_result (nphi),
		       find_edge (seq_start_bb, body_bb), locus);

	  if (!single_pred_p (fin_bb))
	    add_phi_arg (phi, gimple_phi_result (nphi), se, locus);
	}
      gcc_assert (gsi_end_p (psi) && (head == NULL || i == head->length ()));
      redirect_edge_var_map_clear (re);
      if (single_pred_p (fin_bb))
	while (1)
	  {
	    psi = gsi_start_phis (fin_bb);
	    if (gsi_end_p (psi))
	      break;
	    remove_phi_node (&psi, false);
	  }

      /* Make phi node for trip.  */
      phi = create_phi_node (trip_main, iter_part_bb);
      add_phi_arg (phi, trip_back, single_succ_edge (trip_update_bb),
		   UNKNOWN_LOCATION);
      add_phi_arg (phi, trip_init, single_succ_edge (entry_bb),
		   UNKNOWN_LOCATION);
    }

  if (!broken_loop)
    set_immediate_dominator (CDI_DOMINATORS, trip_update_bb, cont_bb);
  set_immediate_dominator (CDI_DOMINATORS, iter_part_bb,
			   recompute_dominator (CDI_DOMINATORS, iter_part_bb));
  set_immediate_dominator (CDI_DOMINATORS, fin_bb,
			   recompute_dominator (CDI_DOMINATORS, fin_bb));
  set_immediate_dominator (CDI_DOMINATORS, seq_start_bb,
			   recompute_dominator (CDI_DOMINATORS, seq_start_bb));
  set_immediate_dominator (CDI_DOMINATORS, body_bb,
			   recompute_dominator (CDI_DOMINATORS, body_bb));

  if (!broken_loop)
    {
      class loop *loop = body_bb->loop_father;
      class loop *trip_loop = alloc_loop ();
      trip_loop->header = iter_part_bb;
      trip_loop->latch = trip_update_bb;
      add_loop (trip_loop, iter_part_bb->loop_father);

      if (loop != entry_bb->loop_father)
	{
	  gcc_assert (loop->header == body_bb);
	  gcc_assert (loop->latch == region->cont
		      || single_pred (loop->latch) == region->cont);
	  trip_loop->inner = loop;
	  return;
	}

      if (!gimple_omp_for_combined_p (fd->for_stmt))
	{
	  loop = alloc_loop ();
	  loop->header = body_bb;
	  if (collapse_bb == NULL)
	    loop->latch = cont_bb;
	  add_loop (loop, trip_loop);
	}
    }


// Source: omp-expand.c
// Lines 4441-5134
