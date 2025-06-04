  size_t m_len;
};

/* Specialization of edit_distance_traits for string_fragment,
   for use by get_closest_sanitizer_option.  */

template <>
struct edit_distance_traits<const string_fragment &>
{
  static size_t get_length (const string_fragment &fragment)
  {
    return fragment.m_len;
  }

  static const char *get_string (const string_fragment &fragment)
  {
    return fragment.m_start;
  }
};

/* Given ARG, an unrecognized sanitizer option, return the best
   matching sanitizer option, or NULL if there isn't one.
   OPTS is array of candidate sanitizer options.
   CODE is OPT_fsanitize_, OPT_fsanitize_recover_ or
   OPT_fsanitize_coverage_.
   VALUE is non-zero for the regular form of the option, zero
   for the "no-" form (e.g. "-fno-sanitize-recover=").  */

static const char *
get_closest_sanitizer_option (const string_fragment &arg,
			      const struct sanitizer_opts_s *opts,
			      enum opt_code code, int value)
{
  best_match <const string_fragment &, const char*> bm (arg);
  for (int i = 0; opts[i].name != NULL; ++i)
    {
      /* -fsanitize=all is not valid, so don't offer it.  */
      if (code == OPT_fsanitize_
	  && opts[i].flag == ~0U
	  && value)
	continue;

      /* For -fsanitize-recover= (and not -fno-sanitize-recover=),
	 don't offer the non-recoverable options.  */
      if (code == OPT_fsanitize_recover_
	  && !opts[i].can_recover
	  && value)
	continue;

      bm.consider (opts[i].name);
    }
  return bm.get_best_meaningful_candidate ();
}

/* Parse comma separated sanitizer suboptions from P for option SCODE,
   adjust previous FLAGS and return new ones.  If COMPLAIN is false,
   don't issue diagnostics.  */

unsigned int
parse_sanitizer_options (const char *p, location_t loc, int scode,
			 unsigned int flags, int value, bool complain)
{
  enum opt_code code = (enum opt_code) scode;

  const struct sanitizer_opts_s *opts;
  if (code == OPT_fsanitize_coverage_)
    opts = coverage_sanitizer_opts;
  else
    opts = sanitizer_opts;

  while (*p != 0)
    {
      size_t len, i;
      bool found = false;
      const char *comma = strchr (p, ',');

      if (comma == NULL)
	len = strlen (p);
      else
	len = comma - p;
      if (len == 0)
	{
	  p = comma + 1;
	  continue;
	}

      /* Check to see if the string matches an option class name.  */
      for (i = 0; opts[i].name != NULL; ++i)
	if (len == opts[i].len && memcmp (p, opts[i].name, len) == 0)
	  {
	    /* Handle both -fsanitize and -fno-sanitize cases.  */
	    if (value && opts[i].flag == ~0U)
	      {
		if (code == OPT_fsanitize_)
		  {
		    if (complain)
		      error_at (loc, "%<-fsanitize=all%> option is not valid");
		  }
		else
		  flags |= ~(SANITIZE_THREAD | SANITIZE_LEAK
			     | SANITIZE_UNREACHABLE | SANITIZE_RETURN);
	      }
	    else if (value)
	      {
		/* Do not enable -fsanitize-recover=unreachable and
		   -fsanitize-recover=return if -fsanitize-recover=undefined
		   is selected.  */
		if (code == OPT_fsanitize_recover_
		    && opts[i].flag == SANITIZE_UNDEFINED)
		  flags |= (SANITIZE_UNDEFINED
			    & ~(SANITIZE_UNREACHABLE | SANITIZE_RETURN));
		else
		  flags |= opts[i].flag;
	      }
	    else
	      flags &= ~opts[i].flag;
	    found = true;
	    break;
	  }

      if (! found && complain)
	{
	  const char *hint
	    = get_closest_sanitizer_option (string_fragment (p, len),
					    opts, code, value);

	  const char *suffix;
	  if (code == OPT_fsanitize_recover_)
	    suffix = "-recover";
	  else if (code == OPT_fsanitize_coverage_)
	    suffix = "-coverage";
	  else
	    suffix = "";

	  if (hint)
	    error_at (loc,
		      "unrecognized argument to %<-f%ssanitize%s=%> "
		      "option: %q.*s; did you mean %qs?",
		      value ? "" : "no-",
		      suffix, (int) len, p, hint);
	  else
	    error_at (loc,
		      "unrecognized argument to %<-f%ssanitize%s=%> option: "
		      "%q.*s", value ? "" : "no-",
		      suffix, (int) len, p);
	}

      if (comma == NULL)
	break;
      p = comma + 1;
    }
  return flags;
}

/* Parse string values of no_sanitize attribute passed in VALUE.
   Values are separated with comma.  */

unsigned int
parse_no_sanitize_attribute (char *value)
{
  unsigned int flags = 0;
  unsigned int i;
  char *q = strtok (value, ",");

  while (q != NULL)
    {
      for (i = 0; sanitizer_opts[i].name != NULL; ++i)
	if (strcmp (sanitizer_opts[i].name, q) == 0)
	  {
	    flags |= sanitizer_opts[i].flag;
	    if (sanitizer_opts[i].flag == SANITIZE_UNDEFINED)
	      flags |= SANITIZE_UNDEFINED_NONDEFAULT;
	    break;
	  }

      if (sanitizer_opts[i].name == NULL)
	warning (OPT_Wattributes,
		 "%qs attribute directive ignored", q);

      q = strtok (NULL, ",");
    }

  return flags;
}

/* Parse -falign-NAME format for a FLAG value.  Return individual
   parsed integer values into RESULT_VALUES array.  If REPORT_ERROR is
   set, print error message at LOC location.  */

bool
parse_and_check_align_values (const char *flag,
			      const char *name,
			      auto_vec<unsigned> &result_values,
			      bool report_error,
			      location_t loc)
{
  char *str = xstrdup (flag);
  for (char *p = strtok (str, ":"); p; p = strtok (NULL, ":"))
    {
      char *end;
      int v = strtol (p, &end, 10);
      if (*end != '\0' || v < 0)
	{
	  if (report_error)
	    error_at (loc, "invalid arguments for %<-falign-%s%> option: %qs",
		      name, flag);

	  return false;
	}

      result_values.safe_push ((unsigned)v);
    }

  free (str);

  /* Check that we have a correct number of values.  */
  if (result_values.is_empty () || result_values.length () > 4)
    {
      if (report_error)
	error_at (loc, "invalid number of arguments for %<-falign-%s%> "
		  "option: %qs", name, flag);
      return false;
    }

  for (unsigned i = 0; i < result_values.length (); i++)
    if (result_values[i] > MAX_CODE_ALIGN_VALUE)
      {
	if (report_error)
	  error_at (loc, "%<-falign-%s%> is not between 0 and %d",
		    name, MAX_CODE_ALIGN_VALUE);
	return false;
      }

  return true;
}

/* Check that alignment value FLAG for -falign-NAME is valid at a given
   location LOC.  */

static void
check_alignment_argument (location_t loc, const char *flag, const char *name)
{
  auto_vec<unsigned> align_result;
  parse_and_check_align_values (flag, name, align_result, true, loc);
}

/* Print help when OPT__help_ is set.  */

void
print_help (struct gcc_options *opts, unsigned int lang_mask,
	    const char *help_option_argument)
{
  const char *a = help_option_argument;
  unsigned int include_flags = 0;
  /* Note - by default we include undocumented options when listing
     specific classes.  If you only want to see documented options
     then add ",^undocumented" to the --help= option.  E.g.:

     --help=target,^undocumented  */
  unsigned int exclude_flags = 0;

  if (lang_mask == CL_DRIVER)
    return;

  /* Walk along the argument string, parsing each word in turn.
     The format is:
     arg = [^]{word}[,{arg}]
     word = {optimizers|target|warnings|undocumented|
     params|common|<language>}  */
  while (*a != 0)
    {
      static const struct
	{
	  const char *string;
	  unsigned int flag;
	}
      specifics[] =
	{
	    { "optimizers", CL_OPTIMIZATION },
	    { "target", CL_TARGET },
	    { "warnings", CL_WARNING },
	    { "undocumented", CL_UNDOCUMENTED },
	    { "params", CL_PARAMS },
	    { "joined", CL_JOINED },
	    { "separate", CL_SEPARATE },
	    { "common", CL_COMMON },
	    { NULL, 0 }
	};
      unsigned int *pflags;
      const char *comma;
      unsigned int lang_flag, specific_flag;
      unsigned int len;
      unsigned int i;

      if (*a == '^')
	{
	  ++a;
	  if (*a == '\0')
	    {
	      error ("missing argument to %qs", "--help=^");
	      break;
	    }
	  pflags = &exclude_flags;
	}
      else
	pflags = &include_flags;

      comma = strchr (a, ',');
      if (comma == NULL)
	len = strlen (a);
      else
	len = comma - a;
      if (len == 0)
	{
	  a = comma + 1;
	  continue;
	}

      /* Check to see if the string matches an option class name.  */
      for (i = 0, specific_flag = 0; specifics[i].string != NULL; i++)
	if (strncasecmp (a, specifics[i].string, len) == 0)
	  {
	    specific_flag = specifics[i].flag;
	    break;
	  }

      /* Check to see if the string matches a language name.
	 Note - we rely upon the alpha-sorted nature of the entries in
	 the lang_names array, specifically that shorter names appear
	 before their longer variants.  (i.e. C before C++).  That way
	 when we are attempting to match --help=c for example we will
	 match with C first and not C++.  */
      for (i = 0, lang_flag = 0; i < cl_lang_count; i++)
	if (strncasecmp (a, lang_names[i], len) == 0)
	  {
	    lang_flag = 1U << i;
	    break;
	  }

      if (specific_flag != 0)
	{
	  if (lang_flag == 0)
	    *pflags |= specific_flag;
	  else
	    {
	      /* The option's argument matches both the start of a
		 language name and the start of an option class name.
		 We have a special case for when the user has
		 specified "--help=c", but otherwise we have to issue
		 a warning.  */
	      if (strncasecmp (a, "c", len) == 0)
		*pflags |= lang_flag;
	      else
		warning (0,
			 "%<--help%> argument %q.*s is ambiguous, "
			 "please be more specific",
			 len, a);
	    }
	}
      else if (lang_flag != 0)
	*pflags |= lang_flag;
      else
	warning (0,
		 "unrecognized argument to %<--help=%> option: %q.*s",
		 len, a);

      if (comma == NULL)
	break;
      a = comma + 1;
    }


// Source: opts.c
// Lines 1804-2173
