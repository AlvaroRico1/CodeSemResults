gimple_fold_builtin_strchr (gimple_stmt_iterator *gsi, bool is_strrchr)
{
  gimple *stmt = gsi_stmt (*gsi);
  tree str = gimple_call_arg (stmt, 0);
  tree c = gimple_call_arg (stmt, 1);
  location_t loc = gimple_location (stmt);
  const char *p;
  char ch;

  if (!gimple_call_lhs (stmt))
    return false;

  /* Avoid folding if the first argument is not a nul-terminated array.
     Defer warning until later.  */
  if (!check_nul_terminated_array (NULL_TREE, str))
    return false;

  if ((p = c_getstr (str)) && target_char_cst_p (c, &ch))
    {
      const char *p1 = is_strrchr ? strrchr (p, ch) : strchr (p, ch);

      if (p1 == NULL)
	{
	  replace_call_with_value (gsi, integer_zero_node);
	  return true;
	}

      tree len = build_int_cst (size_type_node, p1 - p);
      gimple_seq stmts = NULL;
      gimple *new_stmt = gimple_build_assign (gimple_call_lhs (stmt),
					      POINTER_PLUS_EXPR, str, len);
      gimple_seq_add_stmt_without_update (&stmts, new_stmt);
      gsi_replace_with_seq_vops (gsi, stmts);
      return true;
    }


// Source: gimple-fold.c
// Lines 1938-1972
