coverage_init (const char *filename)
{
  const char *original_filename = filename;
  int original_len = strlen (original_filename);
#if HAVE_DOS_BASED_FILE_SYSTEM
  const char *separator = "\\";
#else
  const char *separator = "/";
#endif
  int len = strlen (filename);
  int prefix_len = 0;

  /* Since coverage_init is invoked very early, before the pass
     manager, we need to set up the dumping explicitly. This is
     similar to the handling in finish_optimization_passes.  */
  int profile_pass_num =
    g->get_passes ()->get_pass_profile ()->static_pass_number;
  g->get_dumps ()->dump_start (profile_pass_num, NULL);

  if (!IS_ABSOLUTE_PATH (filename))
    {
      /* When a profile_data_prefix is provided, then mangle full path
	 of filename in order to prevent file path clashing.  */
      if (profile_data_prefix)
	{
	  filename = concat (getpwd (), separator, filename, NULL);
	  if (profile_prefix_path)
	    {
	      if (!strncmp (filename, profile_prefix_path,
			    strlen (profile_prefix_path)))
		{
		  filename += strlen (profile_prefix_path);
		  while (*filename == *separator)
		    filename++;
		}
	      else
		warning (0, "filename %qs does not start with profile "
			 "prefix %qs", filename, profile_prefix_path);
	    }
	  filename = mangle_path (filename);
	  len = strlen (filename);
	}
      else
	profile_data_prefix = getpwd ();
    }

  if (profile_data_prefix)
    prefix_len = strlen (profile_data_prefix);

  /* Name of da file.  */
  da_file_name = XNEWVEC (char, len + strlen (GCOV_DATA_SUFFIX)
			  + prefix_len + 2);

  if (profile_data_prefix)
    {
      memcpy (da_file_name, profile_data_prefix, prefix_len);
      da_file_name[prefix_len++] = *separator;
    }
  memcpy (da_file_name + prefix_len, filename, len);
  strcpy (da_file_name + prefix_len + len, GCOV_DATA_SUFFIX);

  bbg_file_stamp = local_tick;
  
  if (flag_auto_profile)
    read_autofdo_file ();
  else if (flag_branch_probabilities)
    read_counts_file ();

  /* Name of bbg file.  */
  if (flag_test_coverage && !flag_compare_debug)
    {
      if (profile_note_location)
	bbg_file_name = xstrdup (profile_note_location);
      else
	{
	  bbg_file_name = XNEWVEC (char, original_len + strlen (GCOV_NOTE_SUFFIX) + 1);
	  memcpy (bbg_file_name, original_filename, original_len);
	  strcpy (bbg_file_name + original_len, GCOV_NOTE_SUFFIX);
	}

      if (!gcov_open (bbg_file_name, -1))
	{
	  error ("cannot open %s", bbg_file_name);
	  bbg_file_name = NULL;
	}
      else
	{
	  gcov_write_unsigned (GCOV_NOTE_MAGIC);
	  gcov_write_unsigned (GCOV_VERSION);
	  gcov_write_unsigned (bbg_file_stamp);
	  gcov_write_string (getpwd ());

	  /* Do not support has_unexecuted_blocks for Ada.  */
	  gcov_write_unsigned (strcmp (lang_hooks.name, "GNU Ada") != 0);
	}
    }

  g->get_dumps ()->dump_finish (profile_pass_num);
}


// Source: coverage.c
// Lines 1201-1299
