maybe_warn_switch_unreachable (gimple_seq seq)
{
  if (!warn_switch_unreachable
      /* This warning doesn't play well with Fortran when optimizations
	 are on.  */
      || lang_GNU_Fortran ()
      || seq == NULL)
    return;

  struct walk_stmt_info wi;
  memset (&wi, 0, sizeof (wi));
  walk_gimple_seq (seq, warn_switch_unreachable_r, NULL, &wi);
  gimple *stmt = (gimple *) wi.info;

  if (stmt && gimple_code (stmt) != GIMPLE_LABEL)
    {
      if (gimple_code (stmt) == GIMPLE_GOTO
	  && TREE_CODE (gimple_goto_dest (stmt)) == LABEL_DECL
	  && DECL_ARTIFICIAL (gimple_goto_dest (stmt)))
	/* Don't warn for compiler-generated gotos.  These occur
	   in Duff's devices, for example.  */;
      else
	warning_at (gimple_location (stmt), OPT_Wswitch_unreachable,
		    "statement will never be executed");
    }
}


// Source: gimplify.c
// Lines 1940-1965
