set_malloc_flag_1 (cgraph_node *node, bool malloc_p, bool *changed)
{
  if (malloc_p && !DECL_IS_MALLOC (node->decl))
    {
      DECL_IS_MALLOC (node->decl) = true;
      *changed = true;
    }

  ipa_ref *ref;
  FOR_EACH_ALIAS (node, ref)
    {
      cgraph_node *alias = dyn_cast<cgraph_node *> (ref->referring);
      if (!malloc_p || alias->get_availability () > AVAIL_INTERPOSABLE)
	set_malloc_flag_1 (alias, malloc_p, changed);
    }

  for (cgraph_edge *e = node->callers; e; e = e->next_caller)
    if (e->caller->thunk.thunk_p
	&& (!malloc_p || e->caller->get_availability () > AVAIL_INTERPOSABLE))
      set_malloc_flag_1 (e->caller, malloc_p, changed);
}


// Source: cgraph.c
// Lines 2581-2601
