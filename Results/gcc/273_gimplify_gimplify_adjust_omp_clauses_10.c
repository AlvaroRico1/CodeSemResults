gimplify_adjust_omp_clauses (gimple_seq *pre_p, gimple_seq body, tree *list_p,
			     enum tree_code code)
{
  struct gimplify_omp_ctx *ctx = gimplify_omp_ctxp;
  tree *orig_list_p = list_p;
  tree c, decl;
  bool has_inscan_reductions = false;

  if (body)
    {
      struct gimplify_omp_ctx *octx;
      for (octx = ctx; octx; octx = octx->outer_context)
	if ((octx->region_type & (ORT_PARALLEL | ORT_TASK | ORT_TEAMS)) != 0)
	  break;
      if (octx)
	{
	  struct walk_stmt_info wi;
	  memset (&wi, 0, sizeof (wi));
	  walk_gimple_seq (body, omp_find_stores_stmt,
			   omp_find_stores_op, &wi);
	}
    }


// Source: gimplify.c
// Lines 10166-10187
