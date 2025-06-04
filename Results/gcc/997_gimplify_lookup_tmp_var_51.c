lookup_tmp_var (tree val, bool is_formal)
{
  tree ret;

  /* If not optimizing, never really reuse a temporary.  local-alloc
     won't allocate any variable that is used in more than one basic
     block, which means it will go into memory, causing much extra
     work in reload and final and poorer code generation, outweighing
     the extra memory allocation here.  */
  if (!optimize || !is_formal || TREE_SIDE_EFFECTS (val))
    ret = create_tmp_from_val (val);
  else
    {
      elt_t elt, *elt_p;
      elt_t **slot;

      elt.val = val;
      if (!gimplify_ctxp->temp_htab)
        gimplify_ctxp->temp_htab = new hash_table<gimplify_hasher> (1000);
      slot = gimplify_ctxp->temp_htab->find_slot (&elt, INSERT);
      if (*slot == NULL)
	{
	  elt_p = XNEW (elt_t);
	  elt_p->val = val;
	  elt_p->temp = ret = create_tmp_from_val (val);
	  *slot = elt_p;
	}
      else
	{
	  elt_p = *slot;
          ret = elt_p->temp;
	}
    }


// Source: gimplify.c
// Lines 574-606
