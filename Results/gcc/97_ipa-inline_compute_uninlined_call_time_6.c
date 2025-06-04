compute_uninlined_call_time (struct cgraph_edge *edge,
			     sreal uninlined_call_time,
			     sreal freq)
{
  cgraph_node *caller = (edge->caller->inlined_to
			 ? edge->caller->inlined_to
			 : edge->caller);

  if (freq > 0)
    uninlined_call_time *= freq;
  else
    uninlined_call_time = uninlined_call_time >> 11;

  sreal caller_time = ipa_fn_summaries->get (caller)->time;
  return uninlined_call_time + caller_time;
}


// Source: ipa-inline.c
// Lines 728-743
