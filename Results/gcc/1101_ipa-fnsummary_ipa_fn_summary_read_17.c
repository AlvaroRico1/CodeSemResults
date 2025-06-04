ipa_fn_summary_read (void)
{
  struct lto_file_decl_data **file_data_vec = lto_get_file_decl_data ();
  struct lto_file_decl_data *file_data;
  unsigned int j = 0;

  ipa_prop_read_jump_functions ();
  ipa_fn_summary_alloc ();

  while ((file_data = file_data_vec[j++]))
    {
      size_t len;
      const char *data
	= lto_get_summary_section_data (file_data, LTO_section_ipa_fn_summary,
					&len);
      if (data)
	inline_read_section (file_data, data, len);
      else
	/* Fatal error here.  We do not want to support compiling ltrans units
	   with different version of compiler or different flags than the WPA
	   unit, so this should never happen.  */
	fatal_error (input_location,
		     "ipa inline summary is missing in input file");
    }
  ipa_register_cgraph_hooks ();

  gcc_assert (ipa_fn_summaries);
  ipa_fn_summaries->enable_insertion_hook ();
}


// Source: ipa-fnsummary.c
// Lines 4350-4378
