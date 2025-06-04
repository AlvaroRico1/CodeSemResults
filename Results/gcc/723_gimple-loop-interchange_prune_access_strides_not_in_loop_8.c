prune_access_strides_not_in_loop (class loop *loop_nest,
				  class loop *innermost,
				  vec<data_reference_p> datarefs)
{
  data_reference_p dr;
  unsigned num_loops = loop_depth (innermost) - loop_depth (loop_nest) + 1;
  gcc_assert (num_loops > 1);

  /* Block remove strides of loops that is not in current loop nest.  */
  for (unsigned i = 0; datarefs.iterate (i, &dr); ++i)
    {
      vec<tree> *stride = DR_ACCESS_STRIDE (dr);
      if (stride->length () > num_loops)
	stride->block_remove (0, stride->length () - num_loops);
    }


// Source: gimple-loop-interchange.cc
// Lines 1397-1411
