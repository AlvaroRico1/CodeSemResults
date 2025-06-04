omp_notice_threadprivate_variable (struct gimplify_omp_ctx *ctx, tree decl,
				   tree decl2)
{
  splay_tree_node n;
  struct gimplify_omp_ctx *octx;

  for (octx = ctx; octx; octx = octx->outer_context)
    if ((octx->region_type & ORT_TARGET) != 0
	|| octx->order_concurrent)
      {
	n = splay_tree_lookup (octx->variables, (splay_tree_key)decl);
	if (n == NULL)
	  {
	    if (octx->order_concurrent)
	      {
		error ("threadprivate variable %qE used in a region with"
		       " %<order(concurrent)%> clause", DECL_NAME (decl));
		error_at (octx->location, "enclosing region");
	      }
	    else
	      {
		error ("threadprivate variable %qE used in target region",
		       DECL_NAME (decl));
		error_at (octx->location, "enclosing target region");
	      }
	    splay_tree_insert (octx->variables, (splay_tree_key)decl, 0);
	  }
	if (decl2)
	  splay_tree_insert (octx->variables, (splay_tree_key)decl2, 0);
      }

  if (ctx->region_type != ORT_UNTIED_TASK)
    return false;
  n = splay_tree_lookup (ctx->variables, (splay_tree_key)decl);
  if (n == NULL)
    {
      error ("threadprivate variable %qE used in untied task",
	     DECL_NAME (decl));
      error_at (ctx->location, "enclosing task");
      splay_tree_insert (ctx->variables, (splay_tree_key)decl, 0);
    }
  if (decl2)
    splay_tree_insert (ctx->variables, (splay_tree_key)decl2, 0);
  return false;
}


// Source: gimplify.c
// Lines 7094-7138
