sum_callers (struct cgraph_node *node, void *data)
{
  struct cgraph_edge *e;
  int *num_calls = (int *)data;

  for (e = node->callers; e; e = e->next_caller)
    (*num_calls)++;
  return false;
}


// Source: ipa-inline.c
// Lines 1862-1870
