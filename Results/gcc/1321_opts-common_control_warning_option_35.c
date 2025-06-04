control_warning_option (unsigned int opt_index, int kind, const char *arg,
			bool imply, location_t loc, unsigned int lang_mask,
			const struct cl_option_handlers *handlers,
			struct gcc_options *opts,
			struct gcc_options *opts_set,
			diagnostic_context *dc)
{
  if (cl_options[opt_index].alias_target != N_OPTS)
    {
      gcc_assert (!cl_options[opt_index].cl_separate_alias
		  && !cl_options[opt_index].cl_negative_alias);
      if (cl_options[opt_index].alias_arg)
	arg = cl_options[opt_index].alias_arg;
      opt_index = cl_options[opt_index].alias_target;
    }
  if (opt_index == OPT_SPECIAL_ignore || opt_index == OPT_SPECIAL_warn_removed)
    return;
  if (dc)
    diagnostic_classify_diagnostic (dc, opt_index, (diagnostic_t) kind, loc);
  if (imply)
    {
      const struct cl_option *option = &cl_options[opt_index];

      /* -Werror=foo implies -Wfoo.  */
      if (option->var_type == CLVC_BOOLEAN
	  || option->var_type == CLVC_ENUM
	  || option->var_type == CLVC_SIZE)
	{
	  HOST_WIDE_INT value = 1;

	  if (arg && *arg == '\0' && !option->cl_missing_ok)
	    arg = NULL;

	  if ((option->flags & CL_JOINED) && arg == NULL)
	    {
	      cmdline_handle_error (loc, option, option->opt_text, arg,
				    CL_ERR_MISSING_ARG, lang_mask);
	      return;
	    }

	  /* If the switch takes an integer argument, convert it.  */
	  if (arg && (option->cl_uinteger || option->cl_host_wide_int))
	    {
	      int error = 0;
	      value = *arg ? integral_argument (arg, &error,
						option->cl_byte_size) : 0;
	      if (error)
		{
		  cmdline_handle_error (loc, option, option->opt_text, arg,
					CL_ERR_UINT_ARG, lang_mask);
		  return;
		}
	    }


// Source: opts-common.c
// Lines 1661-1713
