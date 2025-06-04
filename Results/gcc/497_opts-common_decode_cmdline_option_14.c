decode_cmdline_option (const char **argv, unsigned int lang_mask,
		       struct cl_decoded_option *decoded)
{
  size_t opt_index;
  const char *arg = 0;
  HOST_WIDE_INT value = 1;
  unsigned int result = 1, i, extra_args, separate_args = 0;
  int adjust_len = 0;
  size_t total_len;
  char *p;
  const struct cl_option *option;
  int errors = 0;
  const char *warn_message = NULL;
  bool separate_arg_flag;
  bool joined_arg_flag;
  bool have_separate_arg = false;

  extra_args = 0;

  const char *opt_value = argv[0] + 1;
  opt_index = find_opt (opt_value, lang_mask);
  i = 0;
  while (opt_index == OPT_SPECIAL_unknown
	 && i < ARRAY_SIZE (option_map))
    {
      const char *opt0 = option_map[i].opt0;
      const char *opt1 = option_map[i].opt1;
      const char *new_prefix = option_map[i].new_prefix;
      bool another_char_needed = option_map[i].another_char_needed;
      size_t opt0_len = strlen (opt0);
      size_t opt1_len = (opt1 == NULL ? 0 : strlen (opt1));
      size_t optn_len = (opt1 == NULL ? opt0_len : opt1_len);
      size_t new_prefix_len = strlen (new_prefix);

      extra_args = (opt1 == NULL ? 0 : 1);
      value = !option_map[i].negated;

      if (strncmp (argv[0], opt0, opt0_len) == 0
	  && (opt1 == NULL
	      || (argv[1] != NULL && strncmp (argv[1], opt1, opt1_len) == 0))
	  && (!another_char_needed
	      || argv[extra_args][optn_len] != 0))
	{
	  size_t arglen = strlen (argv[extra_args]);
	  char *dup;

	  adjust_len = (int) optn_len - (int) new_prefix_len;
	  dup = XNEWVEC (char, arglen + 1 - adjust_len);
	  memcpy (dup, new_prefix, new_prefix_len);
	  memcpy (dup + new_prefix_len, argv[extra_args] + optn_len,
		  arglen - optn_len + 1);
	  opt_index = find_opt (dup + 1, lang_mask);
	  free (dup);
	}
      i++;
    }

  if (opt_index == OPT_SPECIAL_unknown)
    {
      arg = argv[0];
      extra_args = 0;
      value = 1;
      goto done;
    }

  option = &cl_options[opt_index];

  /* Reject negative form of switches that don't take negatives as
     unrecognized.  */
  if (!value && option->cl_reject_negative)
    {
      opt_index = OPT_SPECIAL_unknown;
      errors |= CL_ERR_NEGATIVE;
      arg = argv[0];
      goto done;
    }

  /* Clear the initial value for size options (it will be overwritten
     later based on the Init(value) specification in the opt file.  */
  if (option->var_type == CLVC_SIZE)
    value = 0;

  result = extra_args + 1;
  warn_message = option->warn_message;

  /* Check to see if the option is disabled for this configuration.  */
  if (option->cl_disabled)
    errors |= CL_ERR_DISABLED;

  /* Determine whether there may be a separate argument based on
     whether this option is being processed for the driver, and, if
     so, how many such arguments.  */
  separate_arg_flag = ((option->flags & CL_SEPARATE)
		       && !(option->cl_no_driver_arg
			    && (lang_mask & CL_DRIVER)));
  separate_args = (separate_arg_flag
		   ? option->cl_separate_nargs + 1
		   : 0);
  joined_arg_flag = (option->flags & CL_JOINED) != 0;

  /* Sort out any argument the switch takes.  */
  if (joined_arg_flag)
    {
      /* Have arg point to the original switch.  This is because
	 some code, such as disable_builtin_function, expects its
	 argument to be persistent until the program exits.  */
      arg = argv[extra_args] + cl_options[opt_index].opt_len + 1 + adjust_len;

      if (*arg == '\0' && !option->cl_missing_ok)
	{
	  if (separate_arg_flag)
	    {
	      arg = argv[extra_args + 1];
	      result = extra_args + 2;
	      if (arg == NULL)
		result = extra_args + 1;
	      else
		have_separate_arg = true;
	    }
	  else
	    /* Missing argument.  */
	    arg = NULL;
	}
    }
  else if (separate_arg_flag)
    {
      arg = argv[extra_args + 1];
      for (i = 0; i < separate_args; i++)
	if (argv[extra_args + 1 + i] == NULL)
	  {
	    errors |= CL_ERR_MISSING_ARG;
	    break;
	  }
      result = extra_args + 1 + i;
      if (arg != NULL)
	have_separate_arg = true;
    }

  if (arg == NULL && (separate_arg_flag || joined_arg_flag))
    errors |= CL_ERR_MISSING_ARG;

  /* Is this option an alias (or an ignored option, marked as an alias
     of OPT_SPECIAL_ignore)?  */
  if (option->alias_target != N_OPTS
      && (!option->cl_separate_alias || have_separate_arg))
    {
      size_t new_opt_index = option->alias_target;

      if (new_opt_index == OPT_SPECIAL_ignore
	  || new_opt_index == OPT_SPECIAL_warn_removed)
	{
	  gcc_assert (option->alias_arg == NULL);
	  gcc_assert (option->neg_alias_arg == NULL);
	  opt_index = new_opt_index;
	  arg = NULL;
	}
      else
	{
	  const struct cl_option *new_option = &cl_options[new_opt_index];

	  /* The new option must not be an alias itself.  */
	  gcc_assert (new_option->alias_target == N_OPTS
		      || new_option->cl_separate_alias);

	  if (option->neg_alias_arg)
	    {
	      gcc_assert (option->alias_arg != NULL);
	      gcc_assert (arg == NULL);
	      gcc_assert (!option->cl_negative_alias);
	      if (value)
		arg = option->alias_arg;
	      else
		arg = option->neg_alias_arg;
	      value = 1;
	    }
	  else if (option->alias_arg)
	    {
	      gcc_assert (value == 1);
	      gcc_assert (arg == NULL);
	      gcc_assert (!option->cl_negative_alias);
	      arg = option->alias_arg;
	    }

	  if (option->cl_negative_alias)
	    value = !value;

	  opt_index = new_opt_index;
	  option = new_option;

	  if (value == 0)
	    gcc_assert (!option->cl_reject_negative);

	  /* Recompute what arguments are allowed.  */
	  separate_arg_flag = ((option->flags & CL_SEPARATE)
			       && !(option->cl_no_driver_arg
				    && (lang_mask & CL_DRIVER)));
	  joined_arg_flag = (option->flags & CL_JOINED) != 0;

	  if (separate_args > 1 || option->cl_separate_nargs)
	    gcc_assert (separate_args
			== (unsigned int) option->cl_separate_nargs + 1);

	  if (!(errors & CL_ERR_MISSING_ARG))
	    {
	      if (separate_arg_flag || joined_arg_flag)
		{
		  if (option->cl_missing_ok && arg == NULL)
		    arg = "";
		  gcc_assert (arg != NULL);
		}
	      else
		gcc_assert (arg == NULL);
	    }

	  /* Recheck for warnings and disabled options.  */
	  if (option->warn_message)
	    {
	      gcc_assert (warn_message == NULL);
	      warn_message = option->warn_message;
	    }
	  if (option->cl_disabled)
	    errors |= CL_ERR_DISABLED;
	}
    }

  /* Check if this is a switch for a different front end.  */
  if (!option_ok_for_language (option, lang_mask))
    errors |= CL_ERR_WRONG_LANG;
  else if (strcmp (option->opt_text, "-Werror=") == 0
	   && strchr (opt_value, ',') == NULL)
    {
      /* Verify that -Werror argument is a valid warning
	 for a language.  */
      char *werror_arg = xstrdup (opt_value + 6);
      werror_arg[0] = 'W';

      size_t warning_index = find_opt (werror_arg, lang_mask);
      if (warning_index != OPT_SPECIAL_unknown)
	{
	  const struct cl_option *warning_option
	    = &cl_options[warning_index];
	  if (!option_ok_for_language (warning_option, lang_mask))
	    errors |= CL_ERR_WRONG_LANG;
	}
    }

  /* Convert the argument to lowercase if appropriate.  */
  if (arg && option->cl_tolower)
    {
      size_t j;
      size_t len = strlen (arg);
      char *arg_lower = XOBNEWVEC (&opts_obstack, char, len + 1);

      for (j = 0; j < len; j++)
	arg_lower[j] = TOLOWER ((unsigned char) arg[j]);
      arg_lower[len] = 0;
      arg = arg_lower;
    }

  /* If the switch takes an integer argument, convert it.  */
  if (arg && (option->cl_uinteger || option->cl_host_wide_int))
    {
      int error = 0;
      value = *arg ? integral_argument (arg, &error, option->cl_byte_size) : 0;
      if (error)
	errors |= CL_ERR_UINT_ARG;

      /* Reject value out of a range.  */
      if (option->range_max != -1
	  && (value < option->range_min || value > option->range_max))
	errors |= CL_ERR_INT_RANGE_ARG;
    }

  /* If the switch takes an enumerated argument, convert it.  */
  if (arg && (option->var_type == CLVC_ENUM))
    {
      const struct cl_enum *e = &cl_enums[option->var_enum];

      gcc_assert (value == 1);
      if (enum_arg_to_value (e->values, arg, &value, lang_mask))
	{
	  const char *carg = NULL;

	  if (enum_value_to_arg (e->values, &carg, value, lang_mask))
	    arg = carg;
	  gcc_assert (carg != NULL);
	}
      else
	errors |= CL_ERR_ENUM_ARG;
    }

 done:
  decoded->opt_index = opt_index;
  decoded->arg = arg;
  decoded->value = value;
  decoded->errors = errors;
  decoded->warn_message = warn_message;

  if (opt_index == OPT_SPECIAL_unknown)
    gcc_assert (result == 1);

  gcc_assert (result >= 1 && result <= ARRAY_SIZE (decoded->canonical_option));
  decoded->canonical_option_num_elements = result;
  total_len = 0;
  for (i = 0; i < ARRAY_SIZE (decoded->canonical_option); i++)
    {
      if (i < result)
	{
	  size_t len;
	  if (opt_index == OPT_SPECIAL_unknown)
	    decoded->canonical_option[i] = argv[i];
	  else
	    decoded->canonical_option[i] = NULL;
	  len = strlen (argv[i]);
	  /* If the argument is an empty string, we will print it as "" in
	     orig_option_with_args_text.  */
	  total_len += (len != 0 ? len : 2) + 1;
	}
      else
	decoded->canonical_option[i] = NULL;
    }
  if (opt_index != OPT_SPECIAL_unknown && opt_index != OPT_SPECIAL_ignore
      && opt_index != OPT_SPECIAL_warn_removed)
    {
      generate_canonical_option (opt_index, arg, value, decoded);
      if (separate_args > 1)
	{
	  for (i = 0; i < separate_args; i++)
	    {
	      if (argv[extra_args + 1 + i] == NULL)
		  break;
	      else
		decoded->canonical_option[1 + i] = argv[extra_args + 1 + i];
	    }
	  gcc_assert (result == 1 + i);
	  decoded->canonical_option_num_elements = result;
	}
    }
  decoded->orig_option_with_args_text
    = p = XOBNEWVEC (&opts_obstack, char, total_len);
  for (i = 0; i < result; i++)
    {
      size_t len = strlen (argv[i]);

      /* Print the empty string verbally.  */
      if (len == 0)
	{
	  *p++ = '"';
	  *p++ = '"';
	}
      else
	memcpy (p, argv[i], len);
      p += len;
      if (i == result - 1)
	*p++ = 0;
      else
	*p++ = ' ';
    }

  return result;
}


// Source: opts-common.c
// Lines 532-892
