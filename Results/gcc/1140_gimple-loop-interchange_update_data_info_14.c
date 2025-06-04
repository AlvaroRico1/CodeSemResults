tree_loop_interchange::update_data_info (unsigned i_idx, unsigned o_idx,
					 vec<data_reference_p> datarefs,
					 vec<ddr_p> ddrs)
{
  struct data_reference *dr;
  struct data_dependence_relation *ddr;

  /* Update strides of data references.  */
  for (unsigned i = 0; datarefs.iterate (i, &dr); ++i)
    {
      vec<tree> *stride = DR_ACCESS_STRIDE (dr);
      gcc_assert (stride->length () > i_idx);
      std::swap ((*stride)[i_idx], (*stride)[o_idx]);
    }


// Source: gimple-loop-interchange.cc
// Lines 985-998
