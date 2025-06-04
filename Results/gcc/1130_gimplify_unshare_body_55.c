unshare_body (tree fndecl)
{
  struct cgraph_node *cgn = cgraph_node::get (fndecl);
  /* If the language requires deep unsharing, we need a pointer set to make
     sure we don't repeatedly unshare subtrees of unshareable nodes.  */
  hash_set<tree> *visited
    = lang_hooks.deep_unsharing ? new hash_set<tree> : NULL;

  copy_if_shared (&DECL_SAVED_TREE (fndecl), visited);
  copy_if_shared (&DECL_SIZE (DECL_RESULT (fndecl)), visited);
  copy_if_shared (&DECL_SIZE_UNIT (DECL_RESULT (fndecl)), visited);

  delete visited;

  if (cgn)
    for (cgn = cgn->nested; cgn; cgn = cgn->next_nested)
      unshare_body (cgn->decl);
}


// Source: gimplify.c
// Lines 954-971
