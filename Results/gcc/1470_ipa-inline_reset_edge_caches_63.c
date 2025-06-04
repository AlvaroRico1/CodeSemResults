reset_edge_caches (struct cgraph_node *node)
{
  struct cgraph_edge *edge;
  struct cgraph_edge *e = node->callees;
  struct cgraph_node *where = node;
  struct ipa_ref *ref;

  if (where->inlined_to)
    where = where->inlined_to;

  reset_node_cache (where);

  if (edge_growth_cache != NULL)
    for (edge = where->callers; edge; edge = edge->next_caller)
      if (edge->inline_failed)
	edge_growth_cache->remove (edge);

  FOR_EACH_ALIAS (where, ref)
    reset_edge_caches (dyn_cast <cgraph_node *> (ref->referring));

  if (!e)
    return;

  while (true)
    if (!e->inline_failed && e->callee->callees)
      e = e->callee->callees;
    else
      {
	if (edge_growth_cache != NULL && e->inline_failed)
	  edge_growth_cache->remove (e);
	if (e->next_callee)
	  e = e->next_callee;
	else
	  {
	    do
	      {
		if (e->caller == node)
		  return;
		e = e->caller->callers;
	      }
	    while (!e->next_callee);
	    e = e->next_callee;
	  }
      }
}


// Source: ipa-inline.c
// Lines 1377-1421
