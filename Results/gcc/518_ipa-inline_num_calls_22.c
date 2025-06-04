num_calls (struct cgraph_node *n)
{
  struct cgraph_edge *e;
  int num = 0;

  for (e = n->callees; e; e = e->next_callee)
    if (!is_inexpensive_builtin (e->callee->decl))
      num++;
  return num;
}


// Source: ipa-inline.c
// Lines 629-638
