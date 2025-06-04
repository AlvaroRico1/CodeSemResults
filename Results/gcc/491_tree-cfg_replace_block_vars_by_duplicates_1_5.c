replace_block_vars_by_duplicates_1 (tree *tp, int *walk_subtrees, void *data)
{
  struct replace_decls_d *rd = (struct replace_decls_d *)data;

  switch (TREE_CODE (*tp))
    {
    case VAR_DECL:
    case PARM_DECL:
    case RESULT_DECL:
      replace_by_duplicate_decl (tp, rd->vars_map, rd->to_context);
      break;
    default:
      break;
    }

  if (IS_TYPE_OR_DECL_P (*tp))
    *walk_subtrees = false;

  return NULL;
}


// Source: tree-cfg.c
// Lines 7331-7350
