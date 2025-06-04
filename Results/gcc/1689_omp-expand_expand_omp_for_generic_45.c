expand_omp_for_generic (struct omp_region *region,
			struct omp_for_data *fd,
			enum built_in_function start_fn,
			enum built_in_function next_fn,
			tree sched_arg,
			gimple *inner_stmt)
{
  tree type, istart0, iend0, iend;
  tree t, vmain, vback, bias = NULL_TREE;
  basic_block entry_bb, cont_bb, exit_bb, l0_bb, l1_bb, collapse_bb;
  basic_block l2_bb = NULL, l3_bb = NULL;
  gimple_stmt_iterator gsi;
  gassign *assign_stmt;
  bool in_combined_parallel = is_combined_parallel (region);
  bool broken_loop = region->cont == NULL;
  edge e, ne;
  tree *counts = NULL;
  int i;
  bool ordered_lastprivate = false;

  gcc_assert (!broken_loop || !in_combined_parallel);
  gcc_assert (fd->iter_type == long_integer_type_node
	      || !in_combined_parallel);

  entry_bb = region->entry;
  cont_bb = region->cont;
  collapse_bb = NULL;
  gcc_assert (EDGE_COUNT (entry_bb->succs) == 2);
  gcc_assert (broken_loop
	      || BRANCH_EDGE (entry_bb)->dest == FALLTHRU_EDGE (cont_bb)->dest);
  l0_bb = split_edge (FALLTHRU_EDGE (entry_bb));
  l1_bb = single_succ (l0_bb);
  if (!broken_loop)
    {
      l2_bb = create_empty_bb (cont_bb);
      gcc_assert (BRANCH_EDGE (cont_bb)->dest == l1_bb
		  || (single_succ_edge (BRANCH_EDGE (cont_bb)->dest)->dest
		      == l1_bb));
      gcc_assert (EDGE_COUNT (cont_bb->succs) == 2);
    }
  else
    l2_bb = NULL;
  l3_bb = BRANCH_EDGE (entry_bb)->dest;
  exit_bb = region->exit;

  gsi = gsi_last_nondebug_bb (entry_bb);

  gcc_assert (gimple_code (gsi_stmt (gsi)) == GIMPLE_OMP_FOR);
  if (fd->ordered
      && omp_find_clause (gimple_omp_for_clauses (fd->for_stmt),
			  OMP_CLAUSE_LASTPRIVATE))
    ordered_lastprivate = false;
  tree reductions = NULL_TREE;
  tree mem = NULL_TREE, cond_var = NULL_TREE, condtemp = NULL_TREE;
  tree memv = NULL_TREE;
  if (fd->lastprivate_conditional)
    {
      tree c = omp_find_clause (gimple_omp_for_clauses (fd->for_stmt),
				OMP_CLAUSE__CONDTEMP_);
      if (fd->have_pointer_condtemp)
	condtemp = OMP_CLAUSE_DECL (c);
      c = omp_find_clause (OMP_CLAUSE_CHAIN (c), OMP_CLAUSE__CONDTEMP_);
      cond_var = OMP_CLAUSE_DECL (c);
    }
  if (sched_arg)
    {
      if (fd->have_reductemp)
	{
	  tree c = omp_find_clause (gimple_omp_for_clauses (fd->for_stmt),
				    OMP_CLAUSE__REDUCTEMP_);
	  reductions = OMP_CLAUSE_DECL (c);
	  gcc_assert (TREE_CODE (reductions) == SSA_NAME);
	  gimple *g = SSA_NAME_DEF_STMT (reductions);
	  reductions = gimple_assign_rhs1 (g);
	  OMP_CLAUSE_DECL (c) = reductions;
	  entry_bb = gimple_bb (g);
	  edge e = split_block (entry_bb, g);
	  if (region->entry == entry_bb)
	    region->entry = e->dest;
	  gsi = gsi_last_bb (entry_bb);
	}
      else
	reductions = null_pointer_node;
      if (fd->have_pointer_condtemp)
	{
	  tree type = TREE_TYPE (condtemp);
	  memv = create_tmp_var (type);
	  TREE_ADDRESSABLE (memv) = 1;
	  unsigned HOST_WIDE_INT sz
	    = tree_to_uhwi (TYPE_SIZE_UNIT (TREE_TYPE (type)));
	  sz *= fd->lastprivate_conditional;
	  expand_omp_build_assign (&gsi, memv, build_int_cst (type, sz),
				   false);
	  mem = build_fold_addr_expr (memv);
	}
      else
	mem = null_pointer_node;
    }
  if (fd->collapse > 1 || fd->ordered)
    {
      int first_zero_iter1 = -1, first_zero_iter2 = -1;
      basic_block zero_iter1_bb = NULL, zero_iter2_bb = NULL, l2_dom_bb = NULL;

      counts = XALLOCAVEC (tree, fd->ordered ? fd->ordered + 1 : fd->collapse);
      expand_omp_for_init_counts (fd, &gsi, entry_bb, counts,
				  zero_iter1_bb, first_zero_iter1,
				  zero_iter2_bb, first_zero_iter2, l2_dom_bb);

      if (zero_iter1_bb)
	{
	  /* Some counts[i] vars might be uninitialized if
	     some loop has zero iterations.  But the body shouldn't
	     be executed in that case, so just avoid uninit warnings.  */
	  for (i = first_zero_iter1;
	       i < (fd->ordered ? fd->ordered : fd->collapse); i++)
	    if (SSA_VAR_P (counts[i]))
	      TREE_NO_WARNING (counts[i]) = 1;
	  gsi_prev (&gsi);
	  e = split_block (entry_bb, gsi_stmt (gsi));
	  entry_bb = e->dest;
	  make_edge (zero_iter1_bb, entry_bb, EDGE_FALLTHRU);
	  gsi = gsi_last_nondebug_bb (entry_bb);
	  set_immediate_dominator (CDI_DOMINATORS, entry_bb,
				   get_immediate_dominator (CDI_DOMINATORS,
							    zero_iter1_bb));
	}
      if (zero_iter2_bb)
	{
	  /* Some counts[i] vars might be uninitialized if
	     some loop has zero iterations.  But the body shouldn't
	     be executed in that case, so just avoid uninit warnings.  */
	  for (i = first_zero_iter2; i < fd->ordered; i++)
	    if (SSA_VAR_P (counts[i]))
	      TREE_NO_WARNING (counts[i]) = 1;
	  if (zero_iter1_bb)
	    make_edge (zero_iter2_bb, entry_bb, EDGE_FALLTHRU);
	  else
	    {
	      gsi_prev (&gsi);
	      e = split_block (entry_bb, gsi_stmt (gsi));
	      entry_bb = e->dest;
	      make_edge (zero_iter2_bb, entry_bb, EDGE_FALLTHRU);
	      gsi = gsi_last_nondebug_bb (entry_bb);
	      set_immediate_dominator (CDI_DOMINATORS, entry_bb,
				       get_immediate_dominator
					 (CDI_DOMINATORS, zero_iter2_bb));
	    }
	}
      if (fd->collapse == 1)
	{
	  counts[0] = fd->loop.n2;
	  fd->loop = fd->loops[0];
	}
    }

  type = TREE_TYPE (fd->loop.v);
  istart0 = create_tmp_var (fd->iter_type, ".istart0");
  iend0 = create_tmp_var (fd->iter_type, ".iend0");
  TREE_ADDRESSABLE (istart0) = 1;
  TREE_ADDRESSABLE (iend0) = 1;

  /* See if we need to bias by LLONG_MIN.  */
  if (fd->iter_type == long_long_unsigned_type_node
      && TREE_CODE (type) == INTEGER_TYPE
      && !TYPE_UNSIGNED (type)
      && fd->ordered == 0)
    {
      tree n1, n2;

      if (fd->loop.cond_code == LT_EXPR)
	{
	  n1 = fd->loop.n1;
	  n2 = fold_build2 (PLUS_EXPR, type, fd->loop.n2, fd->loop.step);
	}
      else
	{
	  n1 = fold_build2 (MINUS_EXPR, type, fd->loop.n2, fd->loop.step);
	  n2 = fd->loop.n1;
	}
      if (TREE_CODE (n1) != INTEGER_CST
	  || TREE_CODE (n2) != INTEGER_CST
	  || ((tree_int_cst_sgn (n1) < 0) ^ (tree_int_cst_sgn (n2) < 0)))
	bias = fold_convert (fd->iter_type, TYPE_MIN_VALUE (type));
    }

  gimple_stmt_iterator gsif = gsi;
  gsi_prev (&gsif);

  tree arr = NULL_TREE;
  if (in_combined_parallel)
    {
      gcc_assert (fd->ordered == 0);
      /* In a combined parallel loop, emit a call to
	 GOMP_loop_foo_next.  */
      t = build_call_expr (builtin_decl_explicit (next_fn), 2,
			   build_fold_addr_expr (istart0),
			   build_fold_addr_expr (iend0));
    }
  else
    {
      tree t0, t1, t2, t3, t4;
      /* If this is not a combined parallel loop, emit a call to
	 GOMP_loop_foo_start in ENTRY_BB.  */
      t4 = build_fold_addr_expr (iend0);
      t3 = build_fold_addr_expr (istart0);
      if (fd->ordered)
	{
	  t0 = build_int_cst (unsigned_type_node,
			      fd->ordered - fd->collapse + 1);
	  arr = create_tmp_var (build_array_type_nelts (fd->iter_type,
							fd->ordered
							- fd->collapse + 1),
				".omp_counts");
	  DECL_NAMELESS (arr) = 1;
	  TREE_ADDRESSABLE (arr) = 1;
	  TREE_STATIC (arr) = 1;
	  vec<constructor_elt, va_gc> *v;
	  vec_alloc (v, fd->ordered - fd->collapse + 1);
	  int idx;

	  for (idx = 0; idx < fd->ordered - fd->collapse + 1; idx++)
	    {
	      tree c;
	      if (idx == 0 && fd->collapse > 1)
		c = fd->loop.n2;
	      else
		c = counts[idx + fd->collapse - 1];
	      tree purpose = size_int (idx);
	      CONSTRUCTOR_APPEND_ELT (v, purpose, c);
	      if (TREE_CODE (c) != INTEGER_CST)
		TREE_STATIC (arr) = 0;
	    }

	  DECL_INITIAL (arr) = build_constructor (TREE_TYPE (arr), v);
	  if (!TREE_STATIC (arr))
	    force_gimple_operand_gsi (&gsi, build1 (DECL_EXPR,
						    void_type_node, arr),
				      true, NULL_TREE, true, GSI_SAME_STMT);
	  t1 = build_fold_addr_expr (arr);
	  t2 = NULL_TREE;
	}
      else
	{
	  t2 = fold_convert (fd->iter_type, fd->loop.step);
	  t1 = fd->loop.n2;
	  t0 = fd->loop.n1;
	  if (gimple_omp_for_combined_into_p (fd->for_stmt))
	    {
	      tree innerc
		= omp_find_clause (gimple_omp_for_clauses (fd->for_stmt),
				   OMP_CLAUSE__LOOPTEMP_);
	      gcc_assert (innerc);
	      t0 = OMP_CLAUSE_DECL (innerc);
	      innerc = omp_find_clause (OMP_CLAUSE_CHAIN (innerc),
					OMP_CLAUSE__LOOPTEMP_);
	      gcc_assert (innerc);
	      t1 = OMP_CLAUSE_DECL (innerc);
	    }
	  if (POINTER_TYPE_P (TREE_TYPE (t0))
	      && TYPE_PRECISION (TREE_TYPE (t0))
		 != TYPE_PRECISION (fd->iter_type))
	    {
	      /* Avoid casting pointers to integer of a different size.  */
	      tree itype = signed_type_for (type);
	      t1 = fold_convert (fd->iter_type, fold_convert (itype, t1));
	      t0 = fold_convert (fd->iter_type, fold_convert (itype, t0));
	    }
	  else
	    {
	      t1 = fold_convert (fd->iter_type, t1);
	      t0 = fold_convert (fd->iter_type, t0);
	    }
	  if (bias)
	    {
	      t1 = fold_build2 (PLUS_EXPR, fd->iter_type, t1, bias);
	      t0 = fold_build2 (PLUS_EXPR, fd->iter_type, t0, bias);
	    }
	}
      if (fd->iter_type == long_integer_type_node || fd->ordered)
	{
	  if (fd->chunk_size)
	    {
	      t = fold_convert (fd->iter_type, fd->chunk_size);
	      t = omp_adjust_chunk_size (t, fd->simd_schedule);
	      if (sched_arg)
		{
		  if (fd->ordered)
		    t = build_call_expr (builtin_decl_explicit (start_fn),
					 8, t0, t1, sched_arg, t, t3, t4,
					 reductions, mem);
		  else
		    t = build_call_expr (builtin_decl_explicit (start_fn),
					 9, t0, t1, t2, sched_arg, t, t3, t4,
					 reductions, mem);
		}
	      else if (fd->ordered)
		t = build_call_expr (builtin_decl_explicit (start_fn),
				     5, t0, t1, t, t3, t4);
	      else
		t = build_call_expr (builtin_decl_explicit (start_fn),
				     6, t0, t1, t2, t, t3, t4);
	    }
	  else if (fd->ordered)
	    t = build_call_expr (builtin_decl_explicit (start_fn),
				 4, t0, t1, t3, t4);
	  else
	    t = build_call_expr (builtin_decl_explicit (start_fn),
				 5, t0, t1, t2, t3, t4);
	}
      else
	{
	  tree t5;
	  tree c_bool_type;
	  tree bfn_decl;

	  /* The GOMP_loop_ull_*start functions have additional boolean
	     argument, true for < loops and false for > loops.
	     In Fortran, the C bool type can be different from
	     boolean_type_node.  */
	  bfn_decl = builtin_decl_explicit (start_fn);
	  c_bool_type = TREE_TYPE (TREE_TYPE (bfn_decl));
	  t5 = build_int_cst (c_bool_type,
			      fd->loop.cond_code == LT_EXPR ? 1 : 0);
	  if (fd->chunk_size)
	    {
	      tree bfn_decl = builtin_decl_explicit (start_fn);
	      t = fold_convert (fd->iter_type, fd->chunk_size);
	      t = omp_adjust_chunk_size (t, fd->simd_schedule);
	      if (sched_arg)
		t = build_call_expr (bfn_decl, 10, t5, t0, t1, t2, sched_arg,
				     t, t3, t4, reductions, mem);
	      else
		t = build_call_expr (bfn_decl, 7, t5, t0, t1, t2, t, t3, t4);
	    }
	  else
	    t = build_call_expr (builtin_decl_explicit (start_fn),
				 6, t5, t0, t1, t2, t3, t4);
	}
    }
  if (TREE_TYPE (t) != boolean_type_node)
    t = fold_build2 (NE_EXPR, boolean_type_node,
		     t, build_int_cst (TREE_TYPE (t), 0));
  t = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				true, GSI_SAME_STMT);
  if (arr && !TREE_STATIC (arr))
    {
      tree clobber = build_clobber (TREE_TYPE (arr));
      gsi_insert_before (&gsi, gimple_build_assign (arr, clobber),
			 GSI_SAME_STMT);
    }
  if (fd->have_pointer_condtemp)
    expand_omp_build_assign (&gsi, condtemp, memv, false);
  if (fd->have_reductemp)
    {
      gimple *g = gsi_stmt (gsi);
      gsi_remove (&gsi, true);
      release_ssa_name (gimple_assign_lhs (g));

      entry_bb = region->entry;
      gsi = gsi_last_nondebug_bb (entry_bb);

      gcc_assert (gimple_code (gsi_stmt (gsi)) == GIMPLE_OMP_FOR);
    }
  gsi_insert_after (&gsi, gimple_build_cond_empty (t), GSI_SAME_STMT);

  /* Remove the GIMPLE_OMP_FOR statement.  */
  gsi_remove (&gsi, true);

  if (gsi_end_p (gsif))
    gsif = gsi_after_labels (gsi_bb (gsif));
  gsi_next (&gsif);

  /* Iteration setup for sequential loop goes in L0_BB.  */
  tree startvar = fd->loop.v;
  tree endvar = NULL_TREE;

  if (gimple_omp_for_combined_p (fd->for_stmt))
    {
      gcc_assert (gimple_code (inner_stmt) == GIMPLE_OMP_FOR
		  && gimple_omp_for_kind (inner_stmt)
		     == GF_OMP_FOR_KIND_SIMD);
      tree innerc = omp_find_clause (gimple_omp_for_clauses (inner_stmt),
				     OMP_CLAUSE__LOOPTEMP_);
      gcc_assert (innerc);
      startvar = OMP_CLAUSE_DECL (innerc);
      innerc = omp_find_clause (OMP_CLAUSE_CHAIN (innerc),
				OMP_CLAUSE__LOOPTEMP_);
      gcc_assert (innerc);
      endvar = OMP_CLAUSE_DECL (innerc);
    }

  gsi = gsi_start_bb (l0_bb);
  t = istart0;
  if (fd->ordered && fd->collapse == 1)
    t = fold_build2 (MULT_EXPR, fd->iter_type, t,
		     fold_convert (fd->iter_type, fd->loop.step));
  else if (bias)
    t = fold_build2 (MINUS_EXPR, fd->iter_type, t, bias);
  if (fd->ordered && fd->collapse == 1)
    {
      if (POINTER_TYPE_P (TREE_TYPE (startvar)))
	t = fold_build2 (POINTER_PLUS_EXPR, TREE_TYPE (startvar),
			 fd->loop.n1, fold_convert (sizetype, t));
      else
	{
	  t = fold_convert (TREE_TYPE (startvar), t);
	  t = fold_build2 (PLUS_EXPR, TREE_TYPE (startvar),
			   fd->loop.n1, t);
	}
    }
  else
    {
      if (POINTER_TYPE_P (TREE_TYPE (startvar)))
	t = fold_convert (signed_type_for (TREE_TYPE (startvar)), t);
      t = fold_convert (TREE_TYPE (startvar), t);
    }
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
      if ((fd->ordered && fd->collapse == 1)
	  || bias
	  || POINTER_TYPE_P (type)
	  || TREE_CODE (fd->loop.n1) != INTEGER_CST
	  || fd->loop.cond_code != LT_EXPR)
	t = fold_build2 (PLUS_EXPR, itype, fold_convert (itype, istart0),
			 build_int_cst (itype, 1));
      else if (tree_int_cst_sgn (fd->loop.n1) == 1)
	t = fold_convert (itype, t);
      else
	{
	  tree c = fold_convert (itype, fd->loop.n1);
	  c = fold_build2 (MINUS_EXPR, itype, build_int_cst (itype, 1), c);
	  t = fold_build2 (PLUS_EXPR, itype, fold_convert (itype, t), c);
	}
      t = force_gimple_operand_gsi (&gsi, t, false,
				    NULL_TREE, false, GSI_CONTINUE_LINKING);
      assign_stmt = gimple_build_assign (cond_var, t);
      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
    }

  t = iend0;
  if (fd->ordered && fd->collapse == 1)
    t = fold_build2 (MULT_EXPR, fd->iter_type, t,
		     fold_convert (fd->iter_type, fd->loop.step));
  else if (bias)
    t = fold_build2 (MINUS_EXPR, fd->iter_type, t, bias);
  if (fd->ordered && fd->collapse == 1)
    {
      if (POINTER_TYPE_P (TREE_TYPE (startvar)))
	t = fold_build2 (POINTER_PLUS_EXPR, TREE_TYPE (startvar),
			 fd->loop.n1, fold_convert (sizetype, t));
      else
	{
	  t = fold_convert (TREE_TYPE (startvar), t);
	  t = fold_build2 (PLUS_EXPR, TREE_TYPE (startvar),
			   fd->loop.n1, t);
	}
    }
  else
    {
      if (POINTER_TYPE_P (TREE_TYPE (startvar)))
	t = fold_convert (signed_type_for (TREE_TYPE (startvar)), t);
      t = fold_convert (TREE_TYPE (startvar), t);
    }
  iend = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				   false, GSI_CONTINUE_LINKING);
  if (endvar)
    {
      assign_stmt = gimple_build_assign (endvar, iend);
      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
      if (useless_type_conversion_p (TREE_TYPE (fd->loop.v), TREE_TYPE (iend)))
	assign_stmt = gimple_build_assign (fd->loop.v, iend);
      else
	assign_stmt = gimple_build_assign (fd->loop.v, NOP_EXPR, iend);
      gsi_insert_after (&gsi, assign_stmt, GSI_CONTINUE_LINKING);
    }
  /* Handle linear clause adjustments.  */
  tree itercnt = NULL_TREE;
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
	      itercnt = startvar;
	      tree n1 = fd->loop.n1;
	      if (POINTER_TYPE_P (TREE_TYPE (itercnt)))
		{
		  itercnt
		    = fold_convert (signed_type_for (TREE_TYPE (itercnt)),
				    itercnt);
		  n1 = fold_convert (TREE_TYPE (itercnt), n1);
		}
	      itercnt = fold_build2 (MINUS_EXPR, TREE_TYPE (itercnt),
				     itercnt, n1);
	      itercnt = fold_build2 (EXACT_DIV_EXPR, TREE_TYPE (itercnt),
				     itercnt, fd->loop.step);
	      itercnt = force_gimple_operand_gsi (&gsi, itercnt, true,
						  NULL_TREE, false,
						  GSI_CONTINUE_LINKING);
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

  if (fd->ordered)
    {
      /* Until now, counts array contained number of iterations or
	 variable containing it for ith loop.  From now on, we need
	 those counts only for collapsed loops, and only for the 2nd
	 till the last collapsed one.  Move those one element earlier,
	 we'll use counts[fd->collapse - 1] for the first source/sink
	 iteration counter and so on and counts[fd->ordered]
	 as the array holding the current counter values for
	 depend(source).  */
      if (fd->collapse > 1)
	memmove (counts, counts + 1, (fd->collapse - 1) * sizeof (counts[0]));
      if (broken_loop)
	{
	  int i;
	  for (i = fd->collapse; i < fd->ordered; i++)
	    {
	      tree type = TREE_TYPE (fd->loops[i].v);
	      tree this_cond
		= fold_build2 (fd->loops[i].cond_code, boolean_type_node,
			       fold_convert (type, fd->loops[i].n1),
			       fold_convert (type, fd->loops[i].n2));
	      if (!integer_onep (this_cond))
		break;
	    }
	  if (i < fd->ordered)
	    {
	      cont_bb
		= create_empty_bb (EXIT_BLOCK_PTR_FOR_FN (cfun)->prev_bb);
	      add_bb_to_loop (cont_bb, l1_bb->loop_father);
	      gimple_stmt_iterator gsi = gsi_after_labels (cont_bb);
	      gimple *g = gimple_build_omp_continue (fd->loop.v, fd->loop.v);
	      gsi_insert_before (&gsi, g, GSI_SAME_STMT);
	      make_edge (cont_bb, l3_bb, EDGE_FALLTHRU);
	      make_edge (cont_bb, l1_bb, 0);
	      l2_bb = create_empty_bb (cont_bb);
	      broken_loop = false;
	    }
	}
      expand_omp_ordered_source_sink (region, fd, counts, cont_bb);
      cont_bb = expand_omp_for_ordered_loops (fd, counts, cont_bb, l1_bb,
					      ordered_lastprivate);
      if (counts[fd->collapse - 1])
	{
	  gcc_assert (fd->collapse == 1);
	  gsi = gsi_last_bb (l0_bb);
	  expand_omp_build_assign (&gsi, counts[fd->collapse - 1],
				   istart0, true);
	  if (cont_bb)
	    {
	      gsi = gsi_last_bb (cont_bb);
	      t = fold_build2 (PLUS_EXPR, fd->iter_type,
			       counts[fd->collapse - 1],
			       build_int_cst (fd->iter_type, 1));
	      expand_omp_build_assign (&gsi, counts[fd->collapse - 1], t);
	      tree aref = build4 (ARRAY_REF, fd->iter_type,
				  counts[fd->ordered], size_zero_node,
				  NULL_TREE, NULL_TREE);
	      expand_omp_build_assign (&gsi, aref, counts[fd->collapse - 1]);
	    }
	  t = counts[fd->collapse - 1];
	}
      else if (fd->collapse > 1)
	t = fd->loop.v;
      else
	{
	  t = fold_build2 (MINUS_EXPR, TREE_TYPE (fd->loops[0].v),
			   fd->loops[0].v, fd->loops[0].n1);
	  t = fold_convert (fd->iter_type, t);
	}
      gsi = gsi_last_bb (l0_bb);
      tree aref = build4 (ARRAY_REF, fd->iter_type, counts[fd->ordered],
			  size_zero_node, NULL_TREE, NULL_TREE);
      t = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				    false, GSI_CONTINUE_LINKING);
      expand_omp_build_assign (&gsi, aref, t, true);
    }

  if (!broken_loop)
    {
      /* Code to control the increment and predicate for the sequential
	 loop goes in the CONT_BB.  */
      gsi = gsi_last_nondebug_bb (cont_bb);
      gomp_continue *cont_stmt = as_a <gomp_continue *> (gsi_stmt (gsi));
      gcc_assert (gimple_code (cont_stmt) == GIMPLE_OMP_CONTINUE);
      vmain = gimple_omp_continue_control_use (cont_stmt);
      vback = gimple_omp_continue_control_def (cont_stmt);

      if (cond_var)
	{
	  tree itype = TREE_TYPE (cond_var);
	  tree t2;
	  if ((fd->ordered && fd->collapse == 1)
	       || bias
	       || POINTER_TYPE_P (type)
	       || TREE_CODE (fd->loop.n1) != INTEGER_CST
	       || fd->loop.cond_code != LT_EXPR)
	    t2 = build_int_cst (itype, 1);
	  else
	    t2 = fold_convert (itype, fd->loop.step);
	  t2 = fold_build2 (PLUS_EXPR, itype, cond_var, t2);
	  t2 = force_gimple_operand_gsi (&gsi, t2, false,
					 NULL_TREE, true, GSI_SAME_STMT);
	  assign_stmt = gimple_build_assign (cond_var, t2);
	  gsi_insert_before (&gsi, assign_stmt, GSI_SAME_STMT);
	}

      if (!gimple_omp_for_combined_p (fd->for_stmt))
	{
	  if (POINTER_TYPE_P (type))
	    t = fold_build_pointer_plus (vmain, fd->loop.step);
	  else
	    t = fold_build2 (PLUS_EXPR, type, vmain, fd->loop.step);
	  t = force_gimple_operand_gsi (&gsi, t,
					DECL_P (vback)
					&& TREE_ADDRESSABLE (vback),
					NULL_TREE, true, GSI_SAME_STMT);
	  assign_stmt = gimple_build_assign (vback, t);
	  gsi_insert_before (&gsi, assign_stmt, GSI_SAME_STMT);

	  if (fd->ordered && counts[fd->collapse - 1] == NULL_TREE)
	    {
	      tree tem;
	      if (fd->collapse > 1)
		tem = fd->loop.v;
	      else
		{
		  tem = fold_build2 (MINUS_EXPR, TREE_TYPE (fd->loops[0].v),
				     fd->loops[0].v, fd->loops[0].n1);
		  tem = fold_convert (fd->iter_type, tem);
		}
	      tree aref = build4 (ARRAY_REF, fd->iter_type,
				  counts[fd->ordered], size_zero_node,
				  NULL_TREE, NULL_TREE);
	      tem = force_gimple_operand_gsi (&gsi, tem, true, NULL_TREE,
					      true, GSI_SAME_STMT);
	      expand_omp_build_assign (&gsi, aref, tem);
	    }

	  t = build2 (fd->loop.cond_code, boolean_type_node,
		      DECL_P (vback) && TREE_ADDRESSABLE (vback) ? t : vback,
		      iend);
	  gcond *cond_stmt = gimple_build_cond_empty (t);
	  gsi_insert_before (&gsi, cond_stmt, GSI_SAME_STMT);
	}

      /* Remove GIMPLE_OMP_CONTINUE.  */
      gsi_remove (&gsi, true);

      if (fd->collapse > 1 && !gimple_omp_for_combined_p (fd->for_stmt))
	collapse_bb = extract_omp_for_update_vars (fd, cont_bb, l1_bb);

      /* Emit code to get the next parallel iteration in L2_BB.  */
      gsi = gsi_start_bb (l2_bb);

      t = build_call_expr (builtin_decl_explicit (next_fn), 2,
			   build_fold_addr_expr (istart0),
			   build_fold_addr_expr (iend0));
      t = force_gimple_operand_gsi (&gsi, t, true, NULL_TREE,
				    false, GSI_CONTINUE_LINKING);
      if (TREE_TYPE (t) != boolean_type_node)
	t = fold_build2 (NE_EXPR, boolean_type_node,
			 t, build_int_cst (TREE_TYPE (t), 0));
      gcond *cond_stmt = gimple_build_cond_empty (t);
      gsi_insert_after (&gsi, cond_stmt, GSI_CONTINUE_LINKING);
    }

  /* Add the loop cleanup function.  */
  gsi = gsi_last_nondebug_bb (exit_bb);
  if (gimple_omp_return_nowait_p (gsi_stmt (gsi)))
    t = builtin_decl_explicit (BUILT_IN_GOMP_LOOP_END_NOWAIT);
  else if (gimple_omp_return_lhs (gsi_stmt (gsi)))
    t = builtin_decl_explicit (BUILT_IN_GOMP_LOOP_END_CANCEL);
  else
    t = builtin_decl_explicit (BUILT_IN_GOMP_LOOP_END);
  gcall *call_stmt = gimple_build_call (t, 0);
  if (fd->ordered)
    {
      tree arr = counts[fd->ordered];
      tree clobber = build_clobber (TREE_TYPE (arr));
      gsi_insert_after (&gsi, gimple_build_assign (arr, clobber),
			GSI_SAME_STMT);
    }
  if (gimple_omp_return_lhs (gsi_stmt (gsi)))
    {
      gimple_call_set_lhs (call_stmt, gimple_omp_return_lhs (gsi_stmt (gsi)));
      if (fd->have_reductemp)
	{
	  gimple *g = gimple_build_assign (reductions, NOP_EXPR,
					   gimple_call_lhs (call_stmt));
	  gsi_insert_after (&gsi, g, GSI_SAME_STMT);
	}
    }
  gsi_insert_after (&gsi, call_stmt, GSI_SAME_STMT);
  gsi_remove (&gsi, true);

  /* Connect the new blocks.  */
  find_edge (entry_bb, l0_bb)->flags = EDGE_TRUE_VALUE;
  find_edge (entry_bb, l3_bb)->flags = EDGE_FALSE_VALUE;

  if (!broken_loop)
    {
      gimple_seq phis;

      e = find_edge (cont_bb, l3_bb);
      ne = make_edge (l2_bb, l3_bb, EDGE_FALSE_VALUE);

      phis = phi_nodes (l3_bb);
      for (gsi = gsi_start (phis); !gsi_end_p (gsi); gsi_next (&gsi))
	{
	  gimple *phi = gsi_stmt (gsi);
	  SET_USE (PHI_ARG_DEF_PTR_FROM_EDGE (phi, ne),
		   PHI_ARG_DEF_FROM_EDGE (phi, e));
	}
      remove_edge (e);

      make_edge (cont_bb, l2_bb, EDGE_FALSE_VALUE);
      e = find_edge (cont_bb, l1_bb);
      if (e == NULL)
	{
	  e = BRANCH_EDGE (cont_bb);
	  gcc_assert (single_succ (e->dest) == l1_bb);
	}
      if (gimple_omp_for_combined_p (fd->for_stmt))
	{
	  remove_edge (e);
	  e = NULL;
	}
      else if (fd->collapse > 1)
	{
	  remove_edge (e);
	  e = make_edge (cont_bb, collapse_bb, EDGE_TRUE_VALUE);
	}
      else
	e->flags = EDGE_TRUE_VALUE;
      if (e)
	{
	  e->probability = profile_probability::guessed_always ().apply_scale (7, 8);
	  find_edge (cont_bb, l2_bb)->probability = e->probability.invert ();
	}
      else
	{
	  e = find_edge (cont_bb, l2_bb);
	  e->flags = EDGE_FALLTHRU;
	}
      make_edge (l2_bb, l0_bb, EDGE_TRUE_VALUE);

      if (gimple_in_ssa_p (cfun))
	{
	  /* Add phis to the outer loop that connect to the phis in the inner,
	     original loop, and move the loop entry value of the inner phi to
	     the loop entry value of the outer phi.  */
	  gphi_iterator psi;
	  for (psi = gsi_start_phis (l3_bb); !gsi_end_p (psi); gsi_next (&psi))
	    {
	      location_t locus;
	      gphi *nphi;
	      gphi *exit_phi = psi.phi ();

	      if (virtual_operand_p (gimple_phi_result (exit_phi)))
		continue;

	      edge l2_to_l3 = find_edge (l2_bb, l3_bb);
	      tree exit_res = PHI_ARG_DEF_FROM_EDGE (exit_phi, l2_to_l3);

	      basic_block latch = BRANCH_EDGE (cont_bb)->dest;
	      edge latch_to_l1 = find_edge (latch, l1_bb);
	      gphi *inner_phi
		= find_phi_with_arg_on_edge (exit_res, latch_to_l1);

	      tree t = gimple_phi_result (exit_phi);
	      tree new_res = copy_ssa_name (t, NULL);
	      nphi = create_phi_node (new_res, l0_bb);

	      edge l0_to_l1 = find_edge (l0_bb, l1_bb);
	      t = PHI_ARG_DEF_FROM_EDGE (inner_phi, l0_to_l1);
	      locus = gimple_phi_arg_location_from_edge (inner_phi, l0_to_l1);
	      edge entry_to_l0 = find_edge (entry_bb, l0_bb);
	      add_phi_arg (nphi, t, entry_to_l0, locus);

	      edge l2_to_l0 = find_edge (l2_bb, l0_bb);
	      add_phi_arg (nphi, exit_res, l2_to_l0, UNKNOWN_LOCATION);

	      add_phi_arg (inner_phi, new_res, l0_to_l1, UNKNOWN_LOCATION);
	    }
	}

      set_immediate_dominator (CDI_DOMINATORS, l2_bb,
			       recompute_dominator (CDI_DOMINATORS, l2_bb));
      set_immediate_dominator (CDI_DOMINATORS, l3_bb,
			       recompute_dominator (CDI_DOMINATORS, l3_bb));
      set_immediate_dominator (CDI_DOMINATORS, l0_bb,
			       recompute_dominator (CDI_DOMINATORS, l0_bb));
      set_immediate_dominator (CDI_DOMINATORS, l1_bb,
			       recompute_dominator (CDI_DOMINATORS, l1_bb));

      /* We enter expand_omp_for_generic with a loop.  This original loop may
	 have its own loop struct, or it may be part of an outer loop struct
	 (which may be the fake loop).  */
      class loop *outer_loop = entry_bb->loop_father;
      bool orig_loop_has_loop_struct = l1_bb->loop_father != outer_loop;

      add_bb_to_loop (l2_bb, outer_loop);

      /* We've added a new loop around the original loop.  Allocate the
	 corresponding loop struct.  */
      class loop *new_loop = alloc_loop ();
      new_loop->header = l0_bb;
      new_loop->latch = l2_bb;
      add_loop (new_loop, outer_loop);

      /* Allocate a loop structure for the original loop unless we already
	 had one.  */
      if (!orig_loop_has_loop_struct
	  && !gimple_omp_for_combined_p (fd->for_stmt))
	{
	  class loop *orig_loop = alloc_loop ();
	  orig_loop->header = l1_bb;
	  /* The loop may have multiple latches.  */
	  add_loop (orig_loop, new_loop);
	}
    }


// Source: omp-expand.c
// Lines 2646-3506
