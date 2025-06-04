idx_within_array_bound (tree ref, tree *idx, void *dta)
{
  wi::overflow_type overflow;
  widest_int niter, valid_niter, delta, wi_step;
  tree ev, init, step;
  tree low, high;
  class loop *loop = (class loop*) dta;

  /* Only support within-bound access for array references.  */
  if (TREE_CODE (ref) != ARRAY_REF)
    return false;

  /* For arrays at the end of the structure, we are not guaranteed that they
     do not really extend over their declared size.  However, for arrays of
     size greater than one, this is unlikely to be intended.  */
  if (array_at_struct_end_p (ref))
    return false;

  ev = analyze_scalar_evolution (loop, *idx);
  ev = instantiate_parameters (loop, ev);
  init = initial_condition (ev);
  step = evolution_part_in_loop_num (ev, loop->num);

  if (!init || TREE_CODE (init) != INTEGER_CST
      || (step && TREE_CODE (step) != INTEGER_CST))
    return false;

  low = array_ref_low_bound (ref);
  high = array_ref_up_bound (ref);

  /* The case of nonconstant bounds could be handled, but it would be
     complicated.  */
  if (TREE_CODE (low) != INTEGER_CST
      || !high || TREE_CODE (high) != INTEGER_CST)
    return false;

  /* Check if the intial idx is within bound.  */
  if (wi::to_widest (init) < wi::to_widest (low)
      || wi::to_widest (init) > wi::to_widest (high))
    return false;

  /* The idx is always within bound.  */
  if (!step || integer_zerop (step))
    return true;

  if (!max_loop_iterations (loop, &niter))
    return false;

  if (wi::to_widest (step) < 0)
    {
      delta = wi::to_widest (init) - wi::to_widest (low);
      wi_step = -wi::to_widest (step);
    }
  else
    {
      delta = wi::to_widest (high) - wi::to_widest (init);
      wi_step = wi::to_widest (step);
    }

  valid_niter = wi::div_floor (delta, wi_step, SIGNED, &overflow);
  /* The iteration space of idx is within array bound.  */
  if (!overflow && niter <= valid_niter)
    return true;

  return false;
}


// Source: tree-if-conv.c
// Lines 753-818
