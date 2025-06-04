find_combined_omp_for (tree *tp, int *walk_subtrees, void *data)
{
  tree **pdata = (tree **) data;
  *walk_subtrees = 0;
  switch (TREE_CODE (*tp))
    {
    case OMP_FOR:
      if (OMP_FOR_INIT (*tp) != NULL_TREE)
	{
	  pdata[3] = tp;
	  return *tp;
	}
      pdata[2] = tp;
      *walk_subtrees = 1;
      break;
    case OMP_SIMD:
      if (OMP_FOR_INIT (*tp) != NULL_TREE)
	{
	  pdata[3] = tp;
	  return *tp;
	}
      break;
    case BIND_EXPR:
      if (BIND_EXPR_VARS (*tp)
	  || (BIND_EXPR_BLOCK (*tp)
	      && BLOCK_VARS (BIND_EXPR_BLOCK (*tp))))
	pdata[0] = tp;
      *walk_subtrees = 1;
      break;
    case STATEMENT_LIST:
      if (!tsi_one_before_end_p (tsi_start (*tp)))
	pdata[0] = tp;
      *walk_subtrees = 1;
      break;
    case TRY_FINALLY_EXPR:
      pdata[0] = tp;
      *walk_subtrees = 1;
      break;
    case OMP_PARALLEL:
      pdata[1] = tp;
      *walk_subtrees = 1;
      break;
    default:
      break;
    }
  return NULL_TREE;
}


// Source: omp-general.c
// Lines 515-561
