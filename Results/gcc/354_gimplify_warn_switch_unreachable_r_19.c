warn_switch_unreachable_r (gimple_stmt_iterator *gsi_p, bool *handled_ops_p,
			   struct walk_stmt_info *wi)
{
  gimple *stmt = gsi_stmt (*gsi_p);

  *handled_ops_p = true;
  switch (gimple_code (stmt))
    {
    case GIMPLE_TRY:
      /* A compiler-generated cleanup or a user-written try block.
	 If it's empty, don't dive into it--that would result in
	 worse location info.  */
      if (gimple_try_eval (stmt) == NULL)
	{
	  wi->info = stmt;
	  return integer_zero_node;
	}
      /* Fall through.  */
    case GIMPLE_BIND:
    case GIMPLE_CATCH:
    case GIMPLE_EH_FILTER:
    case GIMPLE_TRANSACTION:
      /* Walk the sub-statements.  */
      *handled_ops_p = false;
      break;

    case GIMPLE_DEBUG:
      /* Ignore these.  We may generate them before declarations that
	 are never executed.  If there's something to warn about,
	 there will be non-debug stmts too, and we'll catch those.  */
      break;

    case GIMPLE_CALL:
      if (gimple_call_internal_p (stmt, IFN_ASAN_MARK))
	{
	  *handled_ops_p = false;
	  break;
	}
      /* Fall through.  */
    default:
      /* Save the first "real" statement (not a decl/lexical scope/...).  */
      wi->info = stmt;
      return integer_zero_node;
    }
  return NULL_TREE;
}


// Source: gimplify.c
// Lines 1889-1934
