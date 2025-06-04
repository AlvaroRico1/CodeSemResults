gimple_add_tmp_var (tree tmp)
{
  gcc_assert (!DECL_CHAIN (tmp) && !DECL_SEEN_IN_BIND_EXPR_P (tmp));

  /* Later processing assumes that the object size is constant, which might
     not be true at this point.  Force the use of a constant upper bound in
     this case.  */
  if (!tree_fits_poly_uint64_p (DECL_SIZE_UNIT (tmp)))
    force_constant_size (tmp);

  DECL_CONTEXT (tmp) = current_function_decl;
  DECL_SEEN_IN_BIND_EXPR_P (tmp) = 1;

  if (gimplify_ctxp)
    {
      DECL_CHAIN (tmp) = gimplify_ctxp->temps;
      gimplify_ctxp->temps = tmp;

      /* Mark temporaries local within the nearest enclosing parallel.  */
      if (gimplify_omp_ctxp)
	{
	  struct gimplify_omp_ctx *ctx = gimplify_omp_ctxp;
	  int flag = GOVD_LOCAL | GOVD_SEEN;
	  while (ctx
		 && (ctx->region_type == ORT_WORKSHARE
		     || ctx->region_type == ORT_TASKGROUP
		     || ctx->region_type == ORT_SIMD
		     || ctx->region_type == ORT_ACC))
	    {
	      if (ctx->region_type == ORT_SIMD
		  && TREE_ADDRESSABLE (tmp)
		  && !TREE_STATIC (tmp))
		{
		  if (TREE_CODE (DECL_SIZE_UNIT (tmp)) != INTEGER_CST)
		    ctx->add_safelen1 = true;
		  else if (ctx->in_for_exprs)
		    flag = GOVD_PRIVATE;
		  else
		    flag = GOVD_PRIVATE | GOVD_SEEN;
		  break;
		}
	      ctx = ctx->outer_context;
	    }
	  if (ctx)
	    omp_add_variable (ctx, tmp, flag);
	}


// Source: gimplify.c
// Lines 765-810
