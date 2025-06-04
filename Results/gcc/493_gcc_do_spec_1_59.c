do_spec_1 (const char *spec, int inswitch, const char *soft_matched_part)
{
  const char *p = spec;
  int c;
  int i;
  int value;

  /* If it's an empty string argument to a switch, keep it as is.  */
  if (inswitch && !*p)
    arg_going = 1;

  while ((c = *p++))
    /* If substituting a switch, treat all chars like letters.
       Otherwise, NL, SPC, TAB and % are special.  */
    switch (inswitch ? 'a' : c)
      {
      case '\n':
	end_going_arg ();

	if (argbuf.length () > 0
	    && !strcmp (argbuf.last (), "|"))
	  {
	    /* A `|' before the newline means use a pipe here,
	       but only if -pipe was specified.
	       Otherwise, execute now and don't pass the `|' as an arg.  */
	    if (use_pipes)
	      {
		input_from_pipe = 1;
		break;
	      }
	    else
	      argbuf.pop ();
	  }

	set_collect_gcc_options ();

	if (argbuf.length () > 0)
	  {
	    value = execute ();
	    if (value)
	      return value;
	  }
	/* Reinitialize for a new command, and for a new argument.  */
	clear_args ();
	arg_going = 0;
	delete_this_arg = 0;
	this_is_output_file = 0;
	this_is_library_file = 0;
	this_is_linker_script = 0;
	input_from_pipe = 0;
	break;

      case '|':
	end_going_arg ();

	/* Use pipe */
	obstack_1grow (&obstack, c);
	arg_going = 1;
	break;

      case '\t':
      case ' ':
	end_going_arg ();

	/* Reinitialize for a new argument.  */
	delete_this_arg = 0;
	this_is_output_file = 0;
	this_is_library_file = 0;
	this_is_linker_script = 0;
	break;

      case '%':
	switch (c = *p++)
	  {
	  case 0:
	    fatal_error (input_location, "spec %qs invalid", spec);

	  case 'b':
	    if (save_temps_length)
	      obstack_grow (&obstack, save_temps_prefix, save_temps_length);
	    else
	      obstack_grow (&obstack, input_basename, basename_length);
	    if (compare_debug < 0)
	      obstack_grow (&obstack, ".gk", 3);
	    arg_going = 1;
	    break;

	  case 'B':
	    if (save_temps_length)
	      obstack_grow (&obstack, save_temps_prefix, save_temps_length);
	    else
	      obstack_grow (&obstack, input_basename, suffixed_basename_length);
	    if (compare_debug < 0)
	      obstack_grow (&obstack, ".gk", 3);
	    arg_going = 1;
	    break;

	  case 'd':
	    delete_this_arg = 2;
	    break;

	  /* Dump out the directories specified with LIBRARY_PATH,
	     followed by the absolute directories
	     that we search for startfiles.  */
	  case 'D':
	    {
	      struct spec_path_info info;

	      info.option = "-L";
	      info.append_len = 0;
#ifdef RELATIVE_PREFIX_NOT_LINKDIR
	      /* Used on systems which record the specified -L dirs
		 and use them to search for dynamic linking.
		 Relative directories always come from -B,
		 and it is better not to use them for searching
		 at run time.  In particular, stage1 loses.  */
	      info.omit_relative = true;
#else
	      info.omit_relative = false;
#endif
	      info.separate_options = false;

	      for_each_path (&startfile_prefixes, true, 0, spec_path, &info);
	    }
	    break;

	  case 'e':
	    /* %efoo means report an error with `foo' as error message
	       and don't execute any more commands for this file.  */
	    {
	      const char *q = p;
	      char *buf;
	      while (*p != 0 && *p != '\n')
		p++;
	      buf = (char *) alloca (p - q + 1);
	      strncpy (buf, q, p - q);
	      buf[p - q] = 0;
	      error ("%s", _(buf));
	      return -1;
	    }
	    break;
	  case 'n':
	    /* %nfoo means report a notice with `foo' on stderr.  */
	    {
	      const char *q = p;
	      char *buf;
	      while (*p != 0 && *p != '\n')
		p++;
	      buf = (char *) alloca (p - q + 1);
	      strncpy (buf, q, p - q);
	      buf[p - q] = 0;
	      inform (UNKNOWN_LOCATION, "%s", _(buf));
	      if (*p)
		p++;
	    }
	    break;

	  case 'j':
	    {
	      struct stat st;

	      /* If save_temps_flag is off, and the HOST_BIT_BUCKET is
		 defined, and it is not a directory, and it is
		 writable, use it.  Otherwise, treat this like any
		 other temporary file.  */

	      if ((!save_temps_flag)
		  && (stat (HOST_BIT_BUCKET, &st) == 0) && (!S_ISDIR (st.st_mode))
		  && (access (HOST_BIT_BUCKET, W_OK) == 0))
		{
		  obstack_grow (&obstack, HOST_BIT_BUCKET,
				strlen (HOST_BIT_BUCKET));
		  delete_this_arg = 0;
		  arg_going = 1;
		  break;
		}
	    }
	    goto create_temp_file;
	  case '|':
	    if (use_pipes)
	      {
		obstack_1grow (&obstack, '-');
		delete_this_arg = 0;
		arg_going = 1;

		/* consume suffix */
		while (*p == '.' || ISALNUM ((unsigned char) *p))
		  p++;
		if (p[0] == '%' && p[1] == 'O')
		  p += 2;

		break;
	      }
	    goto create_temp_file;
	  case 'm':
	    if (use_pipes)
	      {
		/* consume suffix */
		while (*p == '.' || ISALNUM ((unsigned char) *p))
		  p++;
		if (p[0] == '%' && p[1] == 'O')
		  p += 2;

		break;
	      }
	    goto create_temp_file;
	  case 'g':
	  case 'u':
	  case 'U':
	  create_temp_file:
	      {
		struct temp_name *t;
		int suffix_length;
		const char *suffix = p;
		char *saved_suffix = NULL;

		while (*p == '.' || ISALNUM ((unsigned char) *p))
		  p++;
		suffix_length = p - suffix;
		if (p[0] == '%' && p[1] == 'O')
		  {
		    p += 2;
		    /* We don't support extra suffix characters after %O.  */
		    if (*p == '.' || ISALNUM ((unsigned char) *p))
		      fatal_error (input_location,
				   "spec %qs has invalid %<%%0%c%>", spec, *p);
		    if (suffix_length == 0)
		      suffix = TARGET_OBJECT_SUFFIX;
		    else
		      {
			saved_suffix
			  = XNEWVEC (char, suffix_length
				     + strlen (TARGET_OBJECT_SUFFIX) + 1);
			strncpy (saved_suffix, suffix, suffix_length);
			strcpy (saved_suffix + suffix_length,
				TARGET_OBJECT_SUFFIX);
		      }
		    suffix_length += strlen (TARGET_OBJECT_SUFFIX);
		  }

		if (compare_debug < 0)
		  {
		    suffix = concat (".gk", suffix, NULL);
		    suffix_length += 3;
		  }

		/* If -save-temps=obj and -o were specified, use that for the
		   temp file.  */
		if (save_temps_length)
		  {
		    char *tmp;
		    temp_filename_length
		      = save_temps_length + suffix_length + 1;
		    tmp = (char *) alloca (temp_filename_length);
		    memcpy (tmp, save_temps_prefix, save_temps_length);
		    memcpy (tmp + save_temps_length, suffix, suffix_length);
		    tmp[save_temps_length + suffix_length] = '\0';
		    temp_filename = save_string (tmp, save_temps_length
						      + suffix_length);
		    obstack_grow (&obstack, temp_filename,
				  temp_filename_length);
		    arg_going = 1;
		    delete_this_arg = 0;
		    break;
		  }

		/* If the gcc_input_filename has the same suffix specified
		   for the %g, %u, or %U, and -save-temps is specified,
		   we could end up using that file as an intermediate
		   thus clobbering the user's source file (.e.g.,
		   gcc -save-temps foo.s would clobber foo.s with the
		   output of cpp0).  So check for this condition and
		   generate a temp file as the intermediate.  */

		if (save_temps_flag)
		  {
		    char *tmp;
		    temp_filename_length = basename_length + suffix_length + 1;
		    tmp = (char *) alloca (temp_filename_length);
		    memcpy (tmp, input_basename, basename_length);
		    memcpy (tmp + basename_length, suffix, suffix_length);
		    tmp[basename_length + suffix_length] = '\0';
		    temp_filename = tmp;

		    if (filename_cmp (temp_filename, gcc_input_filename) != 0)
		      {
#ifndef HOST_LACKS_INODE_NUMBERS
			struct stat st_temp;

			/* Note, set_input() resets input_stat_set to 0.  */
			if (input_stat_set == 0)
			  {
			    input_stat_set = stat (gcc_input_filename,
						   &input_stat);
			    if (input_stat_set >= 0)
			      input_stat_set = 1;
			  }

			/* If we have the stat for the gcc_input_filename
			   and we can do the stat for the temp_filename
			   then the they could still refer to the same
			   file if st_dev/st_ino's are the same.  */
			if (input_stat_set != 1
			    || stat (temp_filename, &st_temp) < 0
			    || input_stat.st_dev != st_temp.st_dev
			    || input_stat.st_ino != st_temp.st_ino)
#else
			/* Just compare canonical pathnames.  */
			char* input_realname = lrealpath (gcc_input_filename);
			char* temp_realname = lrealpath (temp_filename);
			bool files_differ = filename_cmp (input_realname, temp_realname);
			free (input_realname);
			free (temp_realname);
			if (files_differ)
#endif
			  {
			    temp_filename
			      = save_string (temp_filename,
					     temp_filename_length - 1);
			    obstack_grow (&obstack, temp_filename,
						    temp_filename_length);
			    arg_going = 1;
			    delete_this_arg = 0;
			    break;
			  }
		      }
		  }


// Source: gcc.c
// Lines 5286-5612
