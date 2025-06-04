cgraph_update_edges_for_call_stmt_node (cgraph_node *node,
					gimple *old_stmt, tree old_call,
					gimple *new_stmt)
{
  tree new_call = (new_stmt && is_gimple_call (new_stmt))
		  ? gimple_call_fndecl (new_stmt) : 0;

  /* We are seeing indirect calls, then there is nothing to update.  */
  if (!new_call && !old_call)
    return;
  /* See if we turned indirect call into direct call or folded call to one builtin
     into different builtin.  */
  if (old_call != new_call)
    {
      cgraph_edge *e = node->get_edge (old_stmt);
      cgraph_edge *ne = NULL;
      profile_count count;

      if (e)
	{
	  /* Keep calls marked as dead dead.  */
	  if (new_stmt && is_gimple_call (new_stmt) && e->callee
	      && fndecl_built_in_p (e->callee->decl, BUILT_IN_UNREACHABLE))
	    {
	      cgraph_edge::set_call_stmt (node->get_edge (old_stmt),
					  as_a <gcall *> (new_stmt));
	      return;
	    }
	  /* See if the edge is already there and has the correct callee.  It
	     might be so because of indirect inlining has already updated
	     it.  We also might've cloned and redirected the edge.  */
	  if (new_call && e->callee)
	    {
	      cgraph_node *callee = e->callee;
	      while (callee)
		{
		  if (callee->decl == new_call
		      || callee->former_clone_of == new_call)
		    {
		      cgraph_edge::set_call_stmt (e, as_a <gcall *> (new_stmt));
		      return;
		    }
		  callee = callee->clone_of;
		}
	    }


// Source: cgraph.c
// Lines 1614-1658
