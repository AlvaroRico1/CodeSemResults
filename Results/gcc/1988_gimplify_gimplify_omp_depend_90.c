gimplify_omp_depend (tree *list_p, gimple_seq *pre_p)
{
  tree c;
  gimple *g;
  size_t n[4] = { 0, 0, 0, 0 };
  bool unused[4];
  tree counts[4] = { NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE };
  tree last_iter = NULL_TREE, last_count = NULL_TREE;
  size_t i, j;
  location_t first_loc = UNKNOWN_LOCATION;

  for (c = *list_p; c; c = OMP_CLAUSE_CHAIN (c))
    if (OMP_CLAUSE_CODE (c) == OMP_CLAUSE_DEPEND)
      {
	switch (OMP_CLAUSE_DEPEND_KIND (c))
	  {
	  case OMP_CLAUSE_DEPEND_IN:
	    i = 2;
	    break;
	  case OMP_CLAUSE_DEPEND_OUT:
	  case OMP_CLAUSE_DEPEND_INOUT:
	    i = 0;
	    break;
	  case OMP_CLAUSE_DEPEND_MUTEXINOUTSET:
	    i = 1;
	    break;
	  case OMP_CLAUSE_DEPEND_DEPOBJ:
	    i = 3;
	    break;
	  case OMP_CLAUSE_DEPEND_SOURCE:
	  case OMP_CLAUSE_DEPEND_SINK:
	    continue;
	  default:
	    gcc_unreachable ();
	  }
	tree t = OMP_CLAUSE_DECL (c);
	if (first_loc == UNKNOWN_LOCATION)
	  first_loc = OMP_CLAUSE_LOCATION (c);
	if (TREE_CODE (t) == TREE_LIST
	    && TREE_PURPOSE (t)
	    && TREE_CODE (TREE_PURPOSE (t)) == TREE_VEC)
	  {
	    if (TREE_PURPOSE (t) != last_iter)
	      {
		tree tcnt = size_one_node;
		for (tree it = TREE_PURPOSE (t); it; it = TREE_CHAIN (it))
		  {
		    if (gimplify_expr (&TREE_VEC_ELT (it, 1), pre_p, NULL,
				       is_gimple_val, fb_rvalue) == GS_ERROR
			|| gimplify_expr (&TREE_VEC_ELT (it, 2), pre_p, NULL,
					  is_gimple_val, fb_rvalue) == GS_ERROR
			|| gimplify_expr (&TREE_VEC_ELT (it, 3), pre_p, NULL,
					  is_gimple_val, fb_rvalue) == GS_ERROR
			|| (gimplify_expr (&TREE_VEC_ELT (it, 4), pre_p, NULL,
					   is_gimple_val, fb_rvalue)
			    == GS_ERROR))
		      return 2;
		    tree var = TREE_VEC_ELT (it, 0);
		    tree begin = TREE_VEC_ELT (it, 1);
		    tree end = TREE_VEC_ELT (it, 2);
		    tree step = TREE_VEC_ELT (it, 3);
		    tree orig_step = TREE_VEC_ELT (it, 4);
		    tree type = TREE_TYPE (var);
		    tree stype = TREE_TYPE (step);
		    location_t loc = DECL_SOURCE_LOCATION (var);
		    tree endmbegin;
		    /* Compute count for this iterator as
		       orig_step > 0
		       ? (begin < end ? (end - begin + (step - 1)) / step : 0)
		       : (begin > end ? (end - begin + (step + 1)) / step : 0)
		       and compute product of those for the entire depend
		       clause.  */
		    if (POINTER_TYPE_P (type))
		      endmbegin = fold_build2_loc (loc, POINTER_DIFF_EXPR,
						   stype, end, begin);
		    else
		      endmbegin = fold_build2_loc (loc, MINUS_EXPR, type,
						   end, begin);
		    tree stepm1 = fold_build2_loc (loc, MINUS_EXPR, stype,
						   step,
						   build_int_cst (stype, 1));
		    tree stepp1 = fold_build2_loc (loc, PLUS_EXPR, stype, step,
						   build_int_cst (stype, 1));
		    tree pos = fold_build2_loc (loc, PLUS_EXPR, stype,
						unshare_expr (endmbegin),
						stepm1);
		    pos = fold_build2_loc (loc, TRUNC_DIV_EXPR, stype,
					   pos, step);
		    tree neg = fold_build2_loc (loc, PLUS_EXPR, stype,
						endmbegin, stepp1);
		    if (TYPE_UNSIGNED (stype))
		      {
			neg = fold_build1_loc (loc, NEGATE_EXPR, stype, neg);
			step = fold_build1_loc (loc, NEGATE_EXPR, stype, step);
		      }
		    neg = fold_build2_loc (loc, TRUNC_DIV_EXPR, stype,
					   neg, step);
		    step = NULL_TREE;
		    tree cond = fold_build2_loc (loc, LT_EXPR,
						 boolean_type_node,
						 begin, end);
		    pos = fold_build3_loc (loc, COND_EXPR, stype, cond, pos,
					   build_int_cst (stype, 0));
		    cond = fold_build2_loc (loc, LT_EXPR, boolean_type_node,
					    end, begin);
		    neg = fold_build3_loc (loc, COND_EXPR, stype, cond, neg,
					   build_int_cst (stype, 0));
		    tree osteptype = TREE_TYPE (orig_step);
		    cond = fold_build2_loc (loc, GT_EXPR, boolean_type_node,
					    orig_step,
					    build_int_cst (osteptype, 0));
		    tree cnt = fold_build3_loc (loc, COND_EXPR, stype,
						cond, pos, neg);
		    cnt = fold_convert_loc (loc, sizetype, cnt);
		    if (gimplify_expr (&cnt, pre_p, NULL, is_gimple_val,
				       fb_rvalue) == GS_ERROR)
		      return 2;
		    tcnt = size_binop_loc (loc, MULT_EXPR, tcnt, cnt);
		  }
		if (gimplify_expr (&tcnt, pre_p, NULL, is_gimple_val,
				   fb_rvalue) == GS_ERROR)
		  return 2;
		last_iter = TREE_PURPOSE (t);
		last_count = tcnt;
	      }
	    if (counts[i] == NULL_TREE)
	      counts[i] = last_count;
	    else
	      counts[i] = size_binop_loc (OMP_CLAUSE_LOCATION (c),
					  PLUS_EXPR, counts[i], last_count);
	  }
	else
	  n[i]++;
      }
  for (i = 0; i < 4; i++)
    if (counts[i])
      break;
  if (i == 4)
    return 0;

  tree total = size_zero_node;
  for (i = 0; i < 4; i++)
    {
      unused[i] = counts[i] == NULL_TREE && n[i] == 0;
      if (counts[i] == NULL_TREE)
	counts[i] = size_zero_node;
      if (n[i])
	counts[i] = size_binop (PLUS_EXPR, counts[i], size_int (n[i]));
      if (gimplify_expr (&counts[i], pre_p, NULL, is_gimple_val,
			 fb_rvalue) == GS_ERROR)
	return 2;
      total = size_binop (PLUS_EXPR, total, counts[i]);
    }

  if (gimplify_expr (&total, pre_p, NULL, is_gimple_val, fb_rvalue)
      == GS_ERROR)
    return 2;
  bool is_old = unused[1] && unused[3];
  tree totalpx = size_binop (PLUS_EXPR, unshare_expr (total),
			     size_int (is_old ? 1 : 4));
  tree type = build_array_type (ptr_type_node, build_index_type (totalpx));
  tree array = create_tmp_var_raw (type);
  TREE_ADDRESSABLE (array) = 1;
  if (!poly_int_tree_p (totalpx))
    {
      if (!TYPE_SIZES_GIMPLIFIED (TREE_TYPE (array)))
	gimplify_type_sizes (TREE_TYPE (array), pre_p);
      if (gimplify_omp_ctxp)
	{
	  struct gimplify_omp_ctx *ctx = gimplify_omp_ctxp;
	  while (ctx
		 && (ctx->region_type == ORT_WORKSHARE
		     || ctx->region_type == ORT_TASKGROUP
		     || ctx->region_type == ORT_SIMD
		     || ctx->region_type == ORT_ACC))
	    ctx = ctx->outer_context;
	  if (ctx)
	    omp_add_variable (ctx, array, GOVD_LOCAL | GOVD_SEEN);
	}
      gimplify_vla_decl (array, pre_p);
    }
  else
    gimple_add_tmp_var (array);
  tree r = build4 (ARRAY_REF, ptr_type_node, array, size_int (0), NULL_TREE,
		   NULL_TREE);
  tree tem;
  if (!is_old)
    {
      tem = build2 (MODIFY_EXPR, void_type_node, r,
		    build_int_cst (ptr_type_node, 0));
      gimplify_and_add (tem, pre_p);
      r = build4 (ARRAY_REF, ptr_type_node, array, size_int (1), NULL_TREE,
		  NULL_TREE);
    }
  tem = build2 (MODIFY_EXPR, void_type_node, r,
		fold_convert (ptr_type_node, total));
  gimplify_and_add (tem, pre_p);
  for (i = 1; i < (is_old ? 2 : 4); i++)
    {
      r = build4 (ARRAY_REF, ptr_type_node, array, size_int (i + !is_old),
		  NULL_TREE, NULL_TREE);
      tem = build2 (MODIFY_EXPR, void_type_node, r, counts[i - 1]);
      gimplify_and_add (tem, pre_p);
    }

  tree cnts[4];
  for (j = 4; j; j--)
    if (!unused[j - 1])
      break;
  for (i = 0; i < 4; i++)
    {
      if (i && (i >= j || unused[i - 1]))
	{
	  cnts[i] = cnts[i - 1];
	  continue;
	}
      cnts[i] = create_tmp_var (sizetype);
      if (i == 0)
	g = gimple_build_assign (cnts[i], size_int (is_old ? 2 : 5));
      else
	{
	  tree t;
	  if (is_old)
	    t = size_binop (PLUS_EXPR, counts[0], size_int (2));
	  else
	    t = size_binop (PLUS_EXPR, cnts[i - 1], counts[i - 1]);
	  if (gimplify_expr (&t, pre_p, NULL, is_gimple_val, fb_rvalue)
	      == GS_ERROR)
	    return 2;
	  g = gimple_build_assign (cnts[i], t);
	}
      gimple_seq_add_stmt (pre_p, g);
    }

  last_iter = NULL_TREE;
  tree last_bind = NULL_TREE;
  tree *last_body = NULL;
  for (c = *list_p; c; c = OMP_CLAUSE_CHAIN (c))
    if (OMP_CLAUSE_CODE (c) == OMP_CLAUSE_DEPEND)
      {
	switch (OMP_CLAUSE_DEPEND_KIND (c))
	  {
	  case OMP_CLAUSE_DEPEND_IN:
	    i = 2;
	    break;
	  case OMP_CLAUSE_DEPEND_OUT:
	  case OMP_CLAUSE_DEPEND_INOUT:
	    i = 0;
	    break;
	  case OMP_CLAUSE_DEPEND_MUTEXINOUTSET:
	    i = 1;
	    break;
	  case OMP_CLAUSE_DEPEND_DEPOBJ:
	    i = 3;
	    break;
	  case OMP_CLAUSE_DEPEND_SOURCE:
	  case OMP_CLAUSE_DEPEND_SINK:
	    continue;
	  default:
	    gcc_unreachable ();
	  }
	tree t = OMP_CLAUSE_DECL (c);
	if (TREE_CODE (t) == TREE_LIST
	    && TREE_PURPOSE (t)
	    && TREE_CODE (TREE_PURPOSE (t)) == TREE_VEC)
	  {
	    if (TREE_PURPOSE (t) != last_iter)
	      {
		if (last_bind)
		  gimplify_and_add (last_bind, pre_p);
		tree block = TREE_VEC_ELT (TREE_PURPOSE (t), 5);
		last_bind = build3 (BIND_EXPR, void_type_node,
				    BLOCK_VARS (block), NULL, block);
		TREE_SIDE_EFFECTS (last_bind) = 1;
		SET_EXPR_LOCATION (last_bind, OMP_CLAUSE_LOCATION (c));
		tree *p = &BIND_EXPR_BODY (last_bind);
		for (tree it = TREE_PURPOSE (t); it; it = TREE_CHAIN (it))
		  {
		    tree var = TREE_VEC_ELT (it, 0);
		    tree begin = TREE_VEC_ELT (it, 1);
		    tree end = TREE_VEC_ELT (it, 2);
		    tree step = TREE_VEC_ELT (it, 3);
		    tree orig_step = TREE_VEC_ELT (it, 4);
		    tree type = TREE_TYPE (var);
		    location_t loc = DECL_SOURCE_LOCATION (var);
		    /* Emit:
		       var = begin;
		       goto cond_label;
		       beg_label:
		       ...
		       var = var + step;
		       cond_label:
		       if (orig_step > 0) {
			 if (var < end) goto beg_label;
		       } else {
			 if (var > end) goto beg_label;
		       }
		       for each iterator, with inner iterators added to
		       the ... above.  */
		    tree beg_label = create_artificial_label (loc);
		    tree cond_label = NULL_TREE;
		    tem = build2_loc (loc, MODIFY_EXPR, void_type_node,
				      var, begin);
		    append_to_statement_list_force (tem, p);
		    tem = build_and_jump (&cond_label);
		    append_to_statement_list_force (tem, p);
		    tem = build1 (LABEL_EXPR, void_type_node, beg_label);
		    append_to_statement_list (tem, p);
		    tree bind = build3 (BIND_EXPR, void_type_node, NULL_TREE,
					NULL_TREE, NULL_TREE);
		    TREE_SIDE_EFFECTS (bind) = 1;
		    SET_EXPR_LOCATION (bind, loc);
		    append_to_statement_list_force (bind, p);
		    if (POINTER_TYPE_P (type))
		      tem = build2_loc (loc, POINTER_PLUS_EXPR, type,
					var, fold_convert_loc (loc, sizetype,
							       step));
		    else
		      tem = build2_loc (loc, PLUS_EXPR, type, var, step);
		    tem = build2_loc (loc, MODIFY_EXPR, void_type_node,
				      var, tem);
		    append_to_statement_list_force (tem, p);
		    tem = build1 (LABEL_EXPR, void_type_node, cond_label);
		    append_to_statement_list (tem, p);
		    tree cond = fold_build2_loc (loc, LT_EXPR,
						 boolean_type_node,
						 var, end);
		    tree pos
		      = fold_build3_loc (loc, COND_EXPR, void_type_node,
					 cond, build_and_jump (&beg_label),
					 void_node);
		    cond = fold_build2_loc (loc, GT_EXPR, boolean_type_node,
					    var, end);
		    tree neg
		      = fold_build3_loc (loc, COND_EXPR, void_type_node,
					 cond, build_and_jump (&beg_label),
					 void_node);
		    tree osteptype = TREE_TYPE (orig_step);
		    cond = fold_build2_loc (loc, GT_EXPR, boolean_type_node,
					    orig_step,
					    build_int_cst (osteptype, 0));
		    tem = fold_build3_loc (loc, COND_EXPR, void_type_node,
					   cond, pos, neg);
		    append_to_statement_list_force (tem, p);
		    p = &BIND_EXPR_BODY (bind);
		  }
		last_body = p;
	      }


// Source: gimplify.c
// Lines 7780-8127
