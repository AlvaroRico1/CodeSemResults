verify_node_sharing (tree *tp, int *walk_subtrees, void *data)
{
  struct walk_stmt_info *wi = (struct walk_stmt_info *) data;
  return verify_node_sharing_1 (tp, walk_subtrees, wi->info);
}


// Source: tree-cfg.c
// Lines 5198-5202
