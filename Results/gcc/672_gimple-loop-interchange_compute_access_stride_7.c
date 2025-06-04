compute_access_stride (class loop *loop_nest, class loop *loop,
		       data_reference_p dr)
{
  vec<tree> *strides = new vec<tree> ();
  basic_block bb = gimple_bb (DR_STMT (dr));

  while (!flow_bb_inside_loop_p (loop, bb))
    {
      strides->safe_push (build_int_cst (sizetype, 0));
      loop = loop_outer (loop);
    }
  gcc_assert (loop == bb->loop_father);

  tree ref = DR_REF (dr);
  if (TREE_CODE (ref) == COMPONENT_REF
      && DECL_BIT_FIELD (TREE_OPERAND (ref, 1)))
    {
      /* We can't take address of bitfields.  If the bitfield is at constant
	 offset from the start of the struct, just use address of the
	 struct, for analysis of the strides that shouldn't matter.  */
      if (!TREE_OPERAND (ref, 2)
	  || TREE_CODE (TREE_OPERAND (ref, 2)) == INTEGER_CST)
	ref = TREE_OPERAND (ref, 0);
      /* Otherwise, if we have a bit field representative, use that.  */
      else if (DECL_BIT_FIELD_REPRESENTATIVE (TREE_OPERAND (ref, 1))
	       != NULL_TREE)
	{
	  tree repr = DECL_BIT_FIELD_REPRESENTATIVE (TREE_OPERAND (ref, 1));
	  ref = build3 (COMPONENT_REF, TREE_TYPE (repr), TREE_OPERAND (ref, 0),
			repr, TREE_OPERAND (ref, 2));
	}
      /* Otherwise punt.  */
      else
	{
	  dr->aux = strides;
	  return;
	}
    }
  tree scev_base = build_fold_addr_expr (ref);
  tree scev = analyze_scalar_evolution (loop, scev_base);
  scev = instantiate_scev (loop_preheader_edge (loop_nest), loop, scev);
  if (! chrec_contains_undetermined (scev))
    {
      tree sl = scev;
      class loop *expected = loop;
      while (TREE_CODE (sl) == POLYNOMIAL_CHREC)
	{
	  class loop *sl_loop = get_chrec_loop (sl);
	  while (sl_loop != expected)
	    {
	      strides->safe_push (size_int (0));
	      expected = loop_outer (expected);
	    }
	  strides->safe_push (CHREC_RIGHT (sl));
	  sl = CHREC_LEFT (sl);
	  expected = loop_outer (expected);
	}
      if (! tree_contains_chrecs (sl, NULL))
	while (expected != loop_outer (loop_nest))
	  {
	    strides->safe_push (size_int (0));
	    expected = loop_outer (expected);
	  }
    }


// Source: gimple-loop-interchange.cc
// Lines 1283-1346
