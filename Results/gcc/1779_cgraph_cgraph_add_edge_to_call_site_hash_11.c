cgraph_add_edge_to_call_site_hash (cgraph_edge *e)
{
  /* There are two speculative edges for every statement (one direct,
     one indirect); always hash the direct one.  */
  if (e->speculative && e->indirect_unknown_callee)
    return;
  cgraph_edge **slot = e->caller->call_site_hash->find_slot_with_hash
      (e->call_stmt, cgraph_edge_hasher::hash (e->call_stmt), INSERT);
  if (*slot)
    {
      gcc_assert (((cgraph_edge *)*slot)->speculative);
      if (e->callee && (!e->prev_callee
			|| !e->prev_callee->speculative
			|| e->prev_callee->call_stmt != e->call_stmt))
	*slot = e;
      return;
    }
  gcc_assert (!*slot || e->speculative);
  *slot = e;
}


// Source: cgraph.c
// Lines 707-726
