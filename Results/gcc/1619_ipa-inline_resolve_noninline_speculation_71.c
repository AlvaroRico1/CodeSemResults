resolve_noninline_speculation (edge_heap_t *edge_heap, struct cgraph_edge *edge)
{
  if (edge->speculative && !speculation_useful_p (edge, false))
    {
      struct cgraph_node *node = edge->caller;
      struct cgraph_node *where = node->inlined_to
				  ? node->inlined_to : node;
      auto_bitmap updated_nodes;

      if (edge->count.ipa ().initialized_p ())
        spec_rem += edge->count.ipa ();
      cgraph_edge::resolve_speculation (edge);
      reset_edge_caches (where);
      ipa_update_overall_fn_summary (where);
      update_caller_keys (edge_heap, where,
			  updated_nodes, NULL);
      update_callee_keys (edge_heap, where, NULL,
			  updated_nodes);
    }


// Source: ipa-inline.c
// Lines 1825-1843
