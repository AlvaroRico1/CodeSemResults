compute_access_strides (class loop *loop_nest, class loop *loop,
			vec<data_reference_p> datarefs)
{
  unsigned i, j, num_loops = (unsigned) -1;
  data_reference_p dr;
  vec<tree> *stride;

  for (i = 0; datarefs.iterate (i, &dr); ++i)
    {
      compute_access_stride (loop_nest, loop, dr);
      stride = DR_ACCESS_STRIDE (dr);
      if (stride->length () < num_loops)
	{
	  num_loops = stride->length ();
	  if (num_loops < 2)
	    return NULL;
	}
    }

  for (i = 0; datarefs.iterate (i, &dr); ++i)
    {
      stride = DR_ACCESS_STRIDE (dr);
      if (stride->length () > num_loops)
	stride->truncate (num_loops);

      for (j = 0; j < (num_loops >> 1); ++j)
	std::swap ((*stride)[j], (*stride)[num_loops - j - 1]);
    }

  loop = superloop_at_depth (loop, loop_depth (loop) + 1 - num_loops);
  gcc_assert (loop_nest == loop || flow_loop_nested_p (loop_nest, loop));
  return loop;
}


// Source: gimple-loop-interchange.cc
// Lines 1359-1391
