set_nothrow_flag_1 (cgraph_node *node, bool nothrow, bool non_call,
		    bool *changed)
{
  cgraph_edge *e;

  if (nothrow && !TREE_NOTHROW (node->decl))
    {
      /* With non-call exceptions we can't say for sure if other function body
	 was not possibly optimized to still throw.  */
      if (!non_call || node->binds_to_current_def_p ())
	{
	  TREE_NOTHROW (node->decl) = true;
	  *changed = true;
	  for (e = node->callers; e; e = e->next_caller)
	    e->can_throw_external = false;
	}
    }
  else if (!nothrow && TREE_NOTHROW (node->decl))
    {
      TREE_NOTHROW (node->decl) = false;
      *changed = true;
    }
  ipa_ref *ref;
  FOR_EACH_ALIAS (node, ref)
    {
      cgraph_node *alias = dyn_cast <cgraph_node *> (ref->referring);
      if (!nothrow || alias->get_availability () > AVAIL_INTERPOSABLE)
	set_nothrow_flag_1 (alias, nothrow, non_call, changed);
    }
  for (cgraph_edge *e = node->callers; e; e = e->next_caller)
    if (e->caller->thunk.thunk_p
	&& (!nothrow || e->caller->get_availability () > AVAIL_INTERPOSABLE))
      set_nothrow_flag_1 (e->caller, nothrow, non_call, changed);
}


// Source: cgraph.c
// Lines 2519-2552
