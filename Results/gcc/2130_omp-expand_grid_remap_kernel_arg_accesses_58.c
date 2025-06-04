grid_remap_kernel_arg_accesses (tree *tp, int *walk_subtrees, void *data)
{
  struct walk_stmt_info *wi = (struct walk_stmt_info *) data;
  struct grid_arg_decl_map *adm = (struct grid_arg_decl_map *) wi->info;
  tree t = *tp;

  if (t == adm->old_arg)
    *tp = adm->new_arg;
  *walk_subtrees = !TYPE_P (t) && !DECL_P (t);
  return NULL_TREE;
}


// Source: omp-expand.c
// Lines 8613-8623
