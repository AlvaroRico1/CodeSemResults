maybe_fold_stmt (gimple_stmt_iterator *gsi)
{
  struct gimplify_omp_ctx *ctx;
  for (ctx = gimplify_omp_ctxp; ctx; ctx = ctx->outer_context)
    if ((ctx->region_type & (ORT_TARGET | ORT_PARALLEL | ORT_TASK)) != 0)
      return false;
    else if ((ctx->region_type & ORT_HOST_TEAMS) == ORT_HOST_TEAMS)
      return false;
  /* Delay folding of builtins until the IL is in consistent state
     so the diagnostic machinery can do a better job.  */
  if (gimple_call_builtin_p (gsi_stmt (*gsi)))
    return false;
  return fold_stmt (gsi);
}


// Source: gimplify.c
// Lines 3287-3300
