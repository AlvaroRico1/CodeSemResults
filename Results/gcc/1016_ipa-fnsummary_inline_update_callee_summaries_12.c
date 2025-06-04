inline_update_callee_summaries (struct cgraph_node *node, int depth)
{
  struct cgraph_edge *e;

  ipa_propagate_frequency (node);
  for (e = node->callees; e; e = e->next_callee)
    {
      if (!e->inline_failed)
	inline_update_callee_summaries (e->callee, depth);
      else
	ipa_call_summaries->get (e)->loop_depth += depth;
    }
  for (e = node->indirect_calls; e; e = e->next_callee)
    ipa_call_summaries->get (e)->loop_depth += depth;
}


// Source: ipa-fnsummary.c
// Lines 3731-3745
