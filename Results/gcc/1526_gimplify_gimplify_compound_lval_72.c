gimplify_compound_lval (tree *expr_p, gimple_seq *pre_p, gimple_seq *post_p,
			fallback_t fallback)
{
  tree *p;
  enum gimplify_status ret = GS_ALL_DONE, tret;
  int i;
  location_t loc = EXPR_LOCATION (*expr_p);
  tree expr = *expr_p;

  /* Create a stack of the subexpressions so later we can walk them in
     order from inner to outer.  */
  auto_vec<tree, 10> expr_stack;

  /* We can handle anything that get_inner_reference can deal with.  */
  for (p = expr_p; ; p = &TREE_OPERAND (*p, 0))
    {
    restart:
      /* Fold INDIRECT_REFs now to turn them into ARRAY_REFs.  */
      if (TREE_CODE (*p) == INDIRECT_REF)
	*p = fold_indirect_ref_loc (loc, *p);

      if (handled_component_p (*p))
	;
      /* Expand DECL_VALUE_EXPR now.  In some cases that may expose
	 additional COMPONENT_REFs.  */
      else if ((VAR_P (*p) || TREE_CODE (*p) == PARM_DECL)
	       && gimplify_var_or_parm_decl (p) == GS_OK)
	goto restart;
      else
	break;

      expr_stack.safe_push (*p);
    }

  gcc_assert (expr_stack.length ());

  /* Now EXPR_STACK is a stack of pointers to all the refs we've
     walked through and P points to the innermost expression.

     Java requires that we elaborated nodes in source order.  That
     means we must gimplify the inner expression followed by each of
     the indices, in order.  But we can't gimplify the inner
     expression until we deal with any variable bounds, sizes, or
     positions in order to deal with PLACEHOLDER_EXPRs.

     So we do this in three steps.  First we deal with the annotations
     for any variables in the components, then we gimplify the base,
     then we gimplify any indices, from left to right.  */
  for (i = expr_stack.length () - 1; i >= 0; i--)
    {
      tree t = expr_stack[i];

      if (TREE_CODE (t) == ARRAY_REF || TREE_CODE (t) == ARRAY_RANGE_REF)
	{
	  /* Gimplify the low bound and element type size and put them into
	     the ARRAY_REF.  If these values are set, they have already been
	     gimplified.  */
	  if (TREE_OPERAND (t, 2) == NULL_TREE)
	    {
	      tree low = unshare_expr (array_ref_low_bound (t));
	      if (!is_gimple_min_invariant (low))
		{
		  TREE_OPERAND (t, 2) = low;
		  tret = gimplify_expr (&TREE_OPERAND (t, 2), pre_p,
					post_p, is_gimple_reg,
					fb_rvalue);
		  ret = MIN (ret, tret);
		}
	    }
	  else
	    {
	      tret = gimplify_expr (&TREE_OPERAND (t, 2), pre_p, post_p,
				    is_gimple_reg, fb_rvalue);
	      ret = MIN (ret, tret);
	    }

	  if (TREE_OPERAND (t, 3) == NULL_TREE)
	    {
	      tree elmt_size = array_ref_element_size (t);
	      if (!is_gimple_min_invariant (elmt_size))
		{
		  elmt_size = unshare_expr (elmt_size);
		  tree elmt_type = TREE_TYPE (TREE_TYPE (TREE_OPERAND (t, 0)));
		  tree factor = size_int (TYPE_ALIGN_UNIT (elmt_type));

		  /* Divide the element size by the alignment of the element
		     type (above).  */
		  elmt_size = size_binop_loc (loc, EXACT_DIV_EXPR,
					      elmt_size, factor);

		  TREE_OPERAND (t, 3) = elmt_size;
		  tret = gimplify_expr (&TREE_OPERAND (t, 3), pre_p,
					post_p, is_gimple_reg,
					fb_rvalue);
		  ret = MIN (ret, tret);
		}
	    }
	  else
	    {
	      tret = gimplify_expr (&TREE_OPERAND (t, 3), pre_p, post_p,
				    is_gimple_reg, fb_rvalue);
	      ret = MIN (ret, tret);
	    }
	}
      else if (TREE_CODE (t) == COMPONENT_REF)
	{
	  /* Set the field offset into T and gimplify it.  */
	  if (TREE_OPERAND (t, 2) == NULL_TREE)
	    {
	      tree offset = component_ref_field_offset (t);
	      if (!is_gimple_min_invariant (offset))
		{
		  offset = unshare_expr (offset);
		  tree field = TREE_OPERAND (t, 1);
		  tree factor
		    = size_int (DECL_OFFSET_ALIGN (field) / BITS_PER_UNIT);

		  /* Divide the offset by its alignment.  */
		  offset = size_binop_loc (loc, EXACT_DIV_EXPR,
					   offset, factor);

		  TREE_OPERAND (t, 2) = offset;
		  tret = gimplify_expr (&TREE_OPERAND (t, 2), pre_p,
					post_p, is_gimple_reg,
					fb_rvalue);
		  ret = MIN (ret, tret);
		}
	    }
	  else
	    {
	      tret = gimplify_expr (&TREE_OPERAND (t, 2), pre_p, post_p,
				    is_gimple_reg, fb_rvalue);
	      ret = MIN (ret, tret);
	    }
	}
    }

  /* Step 2 is to gimplify the base expression.  Make sure lvalue is set
     so as to match the min_lval predicate.  Failure to do so may result
     in the creation of large aggregate temporaries.  */
  tret = gimplify_expr (p, pre_p, post_p, is_gimple_min_lval,
			fallback | fb_lvalue);
  ret = MIN (ret, tret);

  /* And finally, the indices and operands of ARRAY_REF.  During this
     loop we also remove any useless conversions.  */
  for (; expr_stack.length () > 0; )
    {
      tree t = expr_stack.pop ();

      if (TREE_CODE (t) == ARRAY_REF || TREE_CODE (t) == ARRAY_RANGE_REF)
	{
	  /* Gimplify the dimension.  */
	  if (!is_gimple_min_invariant (TREE_OPERAND (t, 1)))
	    {
	      tret = gimplify_expr (&TREE_OPERAND (t, 1), pre_p, post_p,
				    is_gimple_val, fb_rvalue);
	      ret = MIN (ret, tret);
	    }
	}

      STRIP_USELESS_TYPE_CONVERSION (TREE_OPERAND (t, 0));

      /* The innermost expression P may have originally had
	 TREE_SIDE_EFFECTS set which would have caused all the outer
	 expressions in *EXPR_P leading to P to also have had
	 TREE_SIDE_EFFECTS set.  */
      recalculate_side_effects (t);
    }

  /* If the outermost expression is a COMPONENT_REF, canonicalize its type.  */
  if ((fallback & fb_rvalue) && TREE_CODE (*expr_p) == COMPONENT_REF)
    {
      canonicalize_component_ref (expr_p);
    }

  expr_stack.release ();

  gcc_assert (*expr_p == expr || ret != GS_ALL_DONE);

  return ret;
}


// Source: gimplify.c
// Lines 2928-3109
