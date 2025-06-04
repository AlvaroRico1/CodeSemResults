unvisit_body (tree fndecl)
{
  struct cgraph_node *cgn = cgraph_node::get (fndecl);

  unmark_visited (&DECL_SAVED_TREE (fndecl));
  unmark_visited (&DECL_SIZE (DECL_RESULT (fndecl)));
  unmark_visited (&DECL_SIZE_UNIT (DECL_RESULT (fndecl)));

  if (cgn)
    for (cgn = cgn->nested; cgn; cgn = cgn->next_nested)
      unvisit_body (cgn->decl);
}


// Source: gimplify.c
// Lines 1003-1014
