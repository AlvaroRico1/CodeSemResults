replace_reduction_placeholders (tree *tp, int *walk_subtrees, void *data)
{
  if (DECL_P (*tp))
    {
      tree *d = (tree *) data;
      if (*tp == OMP_CLAUSE_REDUCTION_PLACEHOLDER (d[0]))
	{
	  *tp = OMP_CLAUSE_REDUCTION_PLACEHOLDER (d[1]);
	  *walk_subtrees = 0;
	}
      else if (*tp == OMP_CLAUSE_REDUCTION_DECL_PLACEHOLDER (d[0]))
	{
	  *tp = OMP_CLAUSE_REDUCTION_DECL_PLACEHOLDER (d[1]);
	  *walk_subtrees = 0;
	}
    }


// Source: gimplify.c
// Lines 12247-12262
