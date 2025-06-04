should_interchange_loops (unsigned i_idx, unsigned o_idx,
			  vec<data_reference_p> datarefs,
			  unsigned i_stmt_cost, unsigned o_stmt_cost,
			  bool innermost_loops_p, bool dump_info_p = true)
{
  unsigned HOST_WIDE_INT ratio;
  unsigned i, j, num_old_inv_drs = 0, num_new_inv_drs = 0;
  struct data_reference *dr;
  bool all_seq_dr_before_p = true, all_seq_dr_after_p = true;
  widest_int iloop_strides = 0, oloop_strides = 0;
  unsigned num_unresolved_drs = 0;
  unsigned num_resolved_ok_drs = 0;
  unsigned num_resolved_not_ok_drs = 0;

  if (dump_info_p && dump_file && (dump_flags & TDF_DETAILS))
    fprintf (dump_file, "\nData ref strides:\n\tmem_ref:\t\tiloop\toloop\n");

  for (i = 0; datarefs.iterate (i, &dr); ++i)
    {
      vec<tree> *stride = DR_ACCESS_STRIDE (dr);
      tree iloop_stride = (*stride)[i_idx], oloop_stride = (*stride)[o_idx];

      bool subloop_stride_p = false;
      /* Data ref can't be invariant or sequential access at current loop if
	 its address changes with respect to any subloops.  */
      for (j = i_idx + 1; j < stride->length (); ++j)
	if (!integer_zerop ((*stride)[j]))
	  {
	    subloop_stride_p = true;
	    break;
	  }

      if (integer_zerop (iloop_stride))
	{
	  if (!subloop_stride_p)
	    num_old_inv_drs++;
	}
      if (integer_zerop (oloop_stride))
	{
	  if (!subloop_stride_p)
	    num_new_inv_drs++;
	}

      if (TREE_CODE (iloop_stride) == INTEGER_CST
	  && TREE_CODE (oloop_stride) == INTEGER_CST)
	{
	  iloop_strides = wi::add (iloop_strides, wi::to_widest (iloop_stride));
	  oloop_strides = wi::add (oloop_strides, wi::to_widest (oloop_stride));
	}
      else if (multiple_of_p (TREE_TYPE (iloop_stride),
			      iloop_stride, oloop_stride))
	num_resolved_ok_drs++;
      else if (multiple_of_p (TREE_TYPE (iloop_stride),
			      oloop_stride, iloop_stride))
	num_resolved_not_ok_drs++;
      else
	num_unresolved_drs++;

      /* Data ref can't be sequential access if its address changes in sub
	 loop.  */
      if (subloop_stride_p)
	{
	  all_seq_dr_before_p = false;
	  all_seq_dr_after_p = false;
	  continue;
	}
      /* Track if all data references are sequential accesses before/after loop
	 interchange.  Note invariant is considered sequential here.  */
      tree access_size = TYPE_SIZE_UNIT (TREE_TYPE (DR_REF (dr)));
      if (all_seq_dr_before_p
	  && ! (integer_zerop (iloop_stride)
		|| operand_equal_p (access_size, iloop_stride, 0)))
	all_seq_dr_before_p = false;
      if (all_seq_dr_after_p
	  && ! (integer_zerop (oloop_stride)
		|| operand_equal_p (access_size, oloop_stride, 0)))
	all_seq_dr_after_p = false;
    }


// Source: gimple-loop-interchange.cc
// Lines 1450-1527
