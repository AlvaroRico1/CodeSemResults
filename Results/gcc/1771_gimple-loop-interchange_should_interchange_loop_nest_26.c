should_interchange_loop_nest (class loop *loop_nest, class loop *innermost,
			      vec<data_reference_p> datarefs)
{
  unsigned idx = loop_depth (innermost) - loop_depth (loop_nest);
  gcc_assert (idx > 0);

  /* Check if any two adjacent loops should be interchanged.  */
  for (class loop *loop = innermost;
       loop != loop_nest; loop = loop_outer (loop), idx--)
    if (should_interchange_loops (idx, idx - 1, datarefs, 0, 0,
				  loop == innermost, false))
      return true;

  return false;
}


// Source: gimple-loop-interchange.cc
// Lines 1818-1832
