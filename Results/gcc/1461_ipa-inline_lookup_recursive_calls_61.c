lookup_recursive_calls (struct cgraph_node *node, struct cgraph_node *where,
			edge_heap_t *heap)
{
  struct cgraph_edge *e;
  enum availability avail;

  for (e = where->callees; e; e = e->next_callee)
    if (e->callee == node
	|| (e->callee->ultimate_alias_target (&avail, e->caller) == node
	    && avail > AVAIL_INTERPOSABLE))
      heap->insert (-e->sreal_frequency (), e);
  for (e = where->callees; e; e = e->next_callee)
    if (!e->inline_failed)
      lookup_recursive_calls (node, e->callee, heap);
}


// Source: ipa-inline.c
// Lines 1563-1577
