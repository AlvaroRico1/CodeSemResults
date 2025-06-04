cgraph_update_edge_in_call_site_hash (cgraph_edge *e)
{
  gimple *call = e->call_stmt;
  *e->caller->call_site_hash->find_slot_with_hash
      (call, cgraph_edge_hasher::hash (call), INSERT) = e;
}


// Source: cgraph.c
// Lines 697-702
