gimple_fold_builtin_strstr (gimple_stmt_iterator *gsi)
{
  gimple *stmt = gsi_stmt (*gsi);
  if (!gimple_call_lhs (stmt))
    return false;

  tree haystack = gimple_call_arg (stmt, 0);
  tree needle = gimple_call_arg (stmt, 1);

  /* Avoid folding if either argument is not a nul-terminated array.
     Defer warning until later.  */
  if (!check_nul_terminated_array (NULL_TREE, haystack)
      || !check_nul_terminated_array (NULL_TREE, needle))
    return false;

  const char *q = c_getstr (needle);
  if (q == NULL)
    return false;

  if (const char *p = c_getstr (haystack))
    {
      const char *r = strstr (p, q);

      if (r == NULL)
	{
	  replace_call_with_value (gsi, integer_zero_node);
	  return true;
	}

      tree len = build_int_cst (size_type_node, r - p);
      gimple_seq stmts = NULL;
      gimple *new_stmt
	= gimple_build_assign (gimple_call_lhs (stmt), POINTER_PLUS_EXPR,
			       haystack, len);
      gimple_seq_add_stmt_without_update (&stmts, new_stmt);
      gsi_replace_with_seq_vops (gsi, stmts);
      return true;
    }


// Source: gimple-fold.c
// Lines 2026-2063
