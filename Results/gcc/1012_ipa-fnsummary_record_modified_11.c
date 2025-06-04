record_modified (ao_ref *ao ATTRIBUTE_UNUSED, tree vdef, void *data)
{
  struct record_modified_bb_info *info =
    (struct record_modified_bb_info *) data;
  if (SSA_NAME_DEF_STMT (vdef) == info->stmt)
    return false;
  if (gimple_clobber_p (SSA_NAME_DEF_STMT (vdef)))
    return false;
  bitmap_set_bit (info->bb_set,
		  SSA_NAME_IS_DEFAULT_DEF (vdef)
		  ? ENTRY_BLOCK_PTR_FOR_FN (cfun)->index
		  : get_minimal_bb
			 (gimple_bb (SSA_NAME_DEF_STMT (vdef)),
			  gimple_bb (info->stmt))->index);
  if (dump_file)
    {
      fprintf (dump_file, "     Param ");
      print_generic_expr (dump_file, info->op, TDF_SLIM);
      fprintf (dump_file, " changed at bb %i, minimal: %i stmt: ",
	       gimple_bb (SSA_NAME_DEF_STMT (vdef))->index,
	       get_minimal_bb
			 (gimple_bb (SSA_NAME_DEF_STMT (vdef)),
			  gimple_bb (info->stmt))->index);
      print_gimple_stmt (dump_file, SSA_NAME_DEF_STMT (vdef), 0);
    }
  return false;
}


// Source: ipa-fnsummary.c
// Lines 2062-2088
