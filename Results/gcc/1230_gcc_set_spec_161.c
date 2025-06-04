set_spec (const char *name, const char *spec, bool user_p)
{
  struct spec_list *sl;
  const char *old_spec;
  int name_len = strlen (name);
  int i;

  /* If this is the first call, initialize the statically allocated specs.  */
  if (!specs)
    {
      struct spec_list *next = (struct spec_list *) 0;
      for (i = ARRAY_SIZE (static_specs) - 1; i >= 0; i--)
	{
	  sl = &static_specs[i];
	  sl->next = next;
	  next = sl;
	}
      specs = sl;
    }

  /* See if the spec already exists.  */
  for (sl = specs; sl; sl = sl->next)
    if (name_len == sl->name_len && !strcmp (sl->name, name))
      break;

  if (!sl)
    {
      /* Not found - make it.  */
      sl = XNEW (struct spec_list);
      sl->name = xstrdup (name);
      sl->name_len = name_len;
      sl->ptr_spec = &sl->ptr;
      sl->alloc_p = 0;
      *(sl->ptr_spec) = "";
      sl->next = specs;
      sl->default_ptr = NULL;
      specs = sl;
    }

  old_spec = *(sl->ptr_spec);
  *(sl->ptr_spec) = ((spec[0] == '+' && ISSPACE ((unsigned char)spec[1]))
		     ? concat (old_spec, spec + 1, NULL)
		     : xstrdup (spec));

#ifdef DEBUG_SPECS
  if (verbose_flag)
    fnotice (stderr, "Setting spec %s to '%s'\n\n", name, *(sl->ptr_spec));
#endif

  /* Free the old spec.  */
  if (old_spec && sl->alloc_p)
    free (CONST_CAST (char *, old_spec));

  sl->user_p = user_p;
  sl->alloc_p = true;
}

/* Accumulate a command (program name and args), and run it.  */

typedef const char *const_char_p; /* For DEF_VEC_P.  */

/* Vector of pointers to arguments in the current line of specifications.  */
static vec<const_char_p> argbuf;

/* Likewise, but for the current @file.  */
static vec<const_char_p> at_file_argbuf;

/* Whether an @file is currently open.  */
static bool in_at_file = false;

/* Were the options -c, -S or -E passed.  */
static int have_c = 0;

/* Was the option -o passed.  */
static int have_o = 0;

/* Was the option -E passed.  */
static int have_E = 0;

/* Pointer to output file name passed in with -o. */
static const char *output_file = 0;

/* This is the list of suffixes and codes (%g/%u/%U/%j) and the associated
   temp file.  If the HOST_BIT_BUCKET is used for %j, no entry is made for
   it here.  */

static struct temp_name {
  const char *suffix;	/* suffix associated with the code.  */
  int length;		/* strlen (suffix).  */
  int unique;		/* Indicates whether %g or %u/%U was used.  */
  const char *filename;	/* associated filename.  */
  int filename_length;	/* strlen (filename).  */
  struct temp_name *next;
} *temp_names;

/* Number of commands executed so far.  */

static int execution_count;

/* Number of commands that exited with a signal.  */

static int signal_count;

/* Allocate the argument vector.  */

static void
alloc_args (void)
{
  argbuf.create (10);
  at_file_argbuf.create (10);
}

/* Clear out the vector of arguments (after a command is executed).  */

static void
clear_args (void)
{
  argbuf.truncate (0);
  at_file_argbuf.truncate (0);
}

/* Add one argument to the vector at the end.
   This is done when a space is seen or at the end of the line.
   If DELETE_ALWAYS is nonzero, the arg is a filename
    and the file should be deleted eventually.
   If DELETE_FAILURE is nonzero, the arg is a filename
    and the file should be deleted if this compilation fails.  */

static void
store_arg (const char *arg, int delete_always, int delete_failure)
{
  if (in_at_file)
    at_file_argbuf.safe_push (arg);
  else
    argbuf.safe_push (arg);

  if (delete_always || delete_failure)
    {
      const char *p;
      /* If the temporary file we should delete is specified as
	 part of a joined argument extract the filename.  */
      if (arg[0] == '-'
	  && (p = strrchr (arg, '=')))
	arg = p + 1;
      record_temp_file (arg, delete_always, delete_failure);
    }
}

/* Open a temporary @file into which subsequent arguments will be stored.  */

static void
open_at_file (void)
{
   if (in_at_file)
     fatal_error (input_location, "cannot open nested response file");
   else
     in_at_file = true;
}

/* Close the temporary @file and add @file to the argument list.  */

static void
close_at_file (void)
{
  if (!in_at_file)
    fatal_error (input_location, "cannot close nonexistent response file");

  in_at_file = false;

  const unsigned int n_args = at_file_argbuf.length ();
  if (n_args == 0)
    return;

  char **argv = (char **) alloca (sizeof (char *) * (n_args + 1));
  char *temp_file = make_temp_file ("");
  char *at_argument = concat ("@", temp_file, NULL);
  FILE *f = fopen (temp_file, "w");
  int status;
  unsigned int i;

  /* Copy the strings over.  */
  for (i = 0; i < n_args; i++)
    argv[i] = CONST_CAST (char *, at_file_argbuf[i]);
  argv[i] = NULL;

  at_file_argbuf.truncate (0);

  if (f == NULL)
    fatal_error (input_location, "could not open temporary response file %s",
		 temp_file);

  status = writeargv (argv, f);

  if (status)
    fatal_error (input_location,
		 "could not write to temporary response file %s",
		 temp_file);

  status = fclose (f);

  if (status == EOF)
    fatal_error (input_location, "could not close temporary response file %s",
		 temp_file);

  store_arg (at_argument, 0, 0);

  record_temp_file (temp_file, !save_temps_flag, !save_temps_flag);
}

/* Load specs from a file name named FILENAME, replacing occurrences of
   various different types of line-endings, \r\n, \n\r and just \r, with
   a single \n.  */

static char *
load_specs (const char *filename)
{
  int desc;
  int readlen;
  struct stat statbuf;
  char *buffer;
  char *buffer_p;
  char *specs;
  char *specs_p;

  if (verbose_flag)
    fnotice (stderr, "Reading specs from %s\n", filename);

  /* Open and stat the file.  */
  desc = open (filename, O_RDONLY, 0);
  if (desc < 0)
    {
    failed:
      /* This leaves DESC open, but the OS will save us.  */
      fatal_error (input_location, "cannot read spec file %qs: %m", filename);
    }

  if (stat (filename, &statbuf) < 0)
    goto failed;

  /* Read contents of file into BUFFER.  */
  buffer = XNEWVEC (char, statbuf.st_size + 1);
  readlen = read (desc, buffer, (unsigned) statbuf.st_size);
  if (readlen < 0)
    goto failed;
  buffer[readlen] = 0;
  close (desc);

  specs = XNEWVEC (char, readlen + 1);
  specs_p = specs;
  for (buffer_p = buffer; buffer_p && *buffer_p; buffer_p++)
    {
      int skip = 0;
      char c = *buffer_p;
      if (c == '\r')
	{
	  if (buffer_p > buffer && *(buffer_p - 1) == '\n')	/* \n\r */
	    skip = 1;
	  else if (*(buffer_p + 1) == '\n')			/* \r\n */
	    skip = 1;
	  else							/* \r */
	    c = '\n';
	}
      if (! skip)
	*specs_p++ = c;
    }
  *specs_p = '\0';

  free (buffer);
  return (specs);
}

/* Read compilation specs from a file named FILENAME,
   replacing the default ones.

   A suffix which starts with `*' is a definition for
   one of the machine-specific sub-specs.  The "suffix" should be
   *asm, *cc1, *cpp, *link, *startfile, etc.
   The corresponding spec is stored in asm_spec, etc.,
   rather than in the `compilers' vector.

   Anything invalid in the file is a fatal error.  */

static void
read_specs (const char *filename, bool main_p, bool user_p)
{
  char *buffer;
  char *p;

  buffer = load_specs (filename);

  /* Scan BUFFER for specs, putting them in the vector.  */
  p = buffer;
  while (1)
    {
      char *suffix;
      char *spec;
      char *in, *out, *p1, *p2, *p3;

      /* Advance P in BUFFER to the next nonblank nocomment line.  */
      p = skip_whitespace (p);
      if (*p == 0)
	break;

      /* Is this a special command that starts with '%'? */
      /* Don't allow this for the main specs file, since it would
	 encourage people to overwrite it.  */
      if (*p == '%' && !main_p)
	{
	  p1 = p;
	  while (*p && *p != '\n')
	    p++;

	  /* Skip '\n'.  */
	  p++;

	  if (!strncmp (p1, "%include", sizeof ("%include") - 1)
	      && (p1[sizeof "%include" - 1] == ' '
		  || p1[sizeof "%include" - 1] == '\t'))
	    {
	      char *new_filename;

	      p1 += sizeof ("%include");
	      while (*p1 == ' ' || *p1 == '\t')
		p1++;

	      if (*p1++ != '<' || p[-2] != '>')
		fatal_error (input_location,
			     "specs %%include syntax malformed after "
			     "%ld characters",
			     (long) (p1 - buffer + 1));

	      p[-2] = '\0';
	      new_filename = find_a_file (&startfile_prefixes, p1, R_OK, true);
	      read_specs (new_filename ? new_filename : p1, false, user_p);
	      continue;
	    }
	  else if (!strncmp (p1, "%include_noerr", sizeof "%include_noerr" - 1)
		   && (p1[sizeof "%include_noerr" - 1] == ' '
		       || p1[sizeof "%include_noerr" - 1] == '\t'))
	    {
	      char *new_filename;

	      p1 += sizeof "%include_noerr";
	      while (*p1 == ' ' || *p1 == '\t')
		p1++;

	      if (*p1++ != '<' || p[-2] != '>')
		fatal_error (input_location,
			     "specs %%include syntax malformed after "
			     "%ld characters",
			     (long) (p1 - buffer + 1));

	      p[-2] = '\0';
	      new_filename = find_a_file (&startfile_prefixes, p1, R_OK, true);
	      if (new_filename)
		read_specs (new_filename, false, user_p);
	      else if (verbose_flag)
		fnotice (stderr, "could not find specs file %s\n", p1);
	      continue;
	    }
	  else if (!strncmp (p1, "%rename", sizeof "%rename" - 1)
		   && (p1[sizeof "%rename" - 1] == ' '
		       || p1[sizeof "%rename" - 1] == '\t'))
	    {
	      int name_len;
	      struct spec_list *sl;
	      struct spec_list *newsl;

	      /* Get original name.  */
	      p1 += sizeof "%rename";
	      while (*p1 == ' ' || *p1 == '\t')
		p1++;

	      if (! ISALPHA ((unsigned char) *p1))
		fatal_error (input_location,
			     "specs %%rename syntax malformed after "
			     "%ld characters",
			     (long) (p1 - buffer));

	      p2 = p1;
	      while (*p2 && !ISSPACE ((unsigned char) *p2))
		p2++;

	      if (*p2 != ' ' && *p2 != '\t')
		fatal_error (input_location,
			     "specs %%rename syntax malformed after "
			     "%ld characters",
			     (long) (p2 - buffer));

	      name_len = p2 - p1;
	      *p2++ = '\0';
	      while (*p2 == ' ' || *p2 == '\t')
		p2++;

	      if (! ISALPHA ((unsigned char) *p2))
		fatal_error (input_location,
			     "specs %%rename syntax malformed after "
			     "%ld characters",
			     (long) (p2 - buffer));

	      /* Get new spec name.  */
	      p3 = p2;
	      while (*p3 && !ISSPACE ((unsigned char) *p3))
		p3++;

	      if (p3 != p - 1)
		fatal_error (input_location,
			     "specs %%rename syntax malformed after "
			     "%ld characters",
			     (long) (p3 - buffer));
	      *p3 = '\0';

	      for (sl = specs; sl; sl = sl->next)
		if (name_len == sl->name_len && !strcmp (sl->name, p1))
		  break;

	      if (!sl)
		fatal_error (input_location,
			     "specs %s spec was not found to be renamed", p1);

	      if (strcmp (p1, p2) == 0)
		continue;

	      for (newsl = specs; newsl; newsl = newsl->next)
		if (strcmp (newsl->name, p2) == 0)
		  fatal_error (input_location,
			       "%s: attempt to rename spec %qs to "
			       "already defined spec %qs",
		    filename, p1, p2);

	      if (verbose_flag)
		{
		  fnotice (stderr, "rename spec %s to %s\n", p1, p2);
#ifdef DEBUG_SPECS
		  fnotice (stderr, "spec is '%s'\n\n", *(sl->ptr_spec));
#endif
		}

	      set_spec (p2, *(sl->ptr_spec), user_p);
	      if (sl->alloc_p)
		free (CONST_CAST (char *, *(sl->ptr_spec)));

	      *(sl->ptr_spec) = "";
	      sl->alloc_p = 0;
	      continue;
	    }
	  else
	    fatal_error (input_location,
			 "specs unknown %% command after %ld characters",
			 (long) (p1 - buffer));
	}

      /* Find the colon that should end the suffix.  */
      p1 = p;
      while (*p1 && *p1 != ':' && *p1 != '\n')
	p1++;

      /* The colon shouldn't be missing.  */
      if (*p1 != ':')
	fatal_error (input_location,
		     "specs file malformed after %ld characters",
		     (long) (p1 - buffer));

      /* Skip back over trailing whitespace.  */
      p2 = p1;
      while (p2 > buffer && (p2[-1] == ' ' || p2[-1] == '\t'))
	p2--;

      /* Copy the suffix to a string.  */
      suffix = save_string (p, p2 - p);
      /* Find the next line.  */
      p = skip_whitespace (p1 + 1);
      if (p[1] == 0)
	fatal_error (input_location,
		     "specs file malformed after %ld characters",
		     (long) (p - buffer));

      p1 = p;
      /* Find next blank line or end of string.  */
      while (*p1 && !(*p1 == '\n' && (p1[1] == '\n' || p1[1] == '\0')))
	p1++;

      /* Specs end at the blank line and do not include the newline.  */
      spec = save_string (p, p1 - p);
      p = p1;

      /* Delete backslash-newline sequences from the spec.  */
      in = spec;
      out = spec;
      while (*in != 0)
	{
	  if (in[0] == '\\' && in[1] == '\n')
	    in += 2;
	  else if (in[0] == '#')
	    while (*in && *in != '\n')
	      in++;

	  else
	    *out++ = *in++;
	}
      *out = 0;

      if (suffix[0] == '*')
	{
	  if (! strcmp (suffix, "*link_command"))
	    link_command_spec = spec;
	  else
	    {
	      set_spec (suffix + 1, spec, user_p);
	      free (spec);
	    }
	}
      else
	{
	  /* Add this pair to the vector.  */
	  compilers
	    = XRESIZEVEC (struct compiler, compilers, n_compilers + 2);

	  compilers[n_compilers].suffix = suffix;
	  compilers[n_compilers].spec = spec;
	  n_compilers++;
	  memset (&compilers[n_compilers], 0, sizeof compilers[n_compilers]);
	}

      if (*suffix == 0)
	link_command_spec = spec;
    }

  if (link_command_spec == 0)
    fatal_error (input_location, "spec file has no spec for linking");

  XDELETEVEC (buffer);
}

/* Record the names of temporary files we tell compilers to write,
   and delete them at the end of the run.  */

/* This is the common prefix we use to make temp file names.
   It is chosen once for each run of this program.
   It is substituted into a spec by %g or %j.
   Thus, all temp file names contain this prefix.
   In practice, all temp file names start with this prefix.

   This prefix comes from the envvar TMPDIR if it is defined;
   otherwise, from the P_tmpdir macro if that is defined;
   otherwise, in /usr/tmp or /tmp;
   or finally the current directory if all else fails.  */

static const char *temp_filename;

/* Length of the prefix.  */

static int temp_filename_length;

/* Define the list of temporary files to delete.  */

struct temp_file
{
  const char *name;
  struct temp_file *next;
};

/* Queue of files to delete on success or failure of compilation.  */
static struct temp_file *always_delete_queue;
/* Queue of files to delete on failure of compilation.  */
static struct temp_file *failure_delete_queue;

/* Record FILENAME as a file to be deleted automatically.
   ALWAYS_DELETE nonzero means delete it if all compilation succeeds;
   otherwise delete it in any case.
   FAIL_DELETE nonzero means delete it if a compilation step fails;
   otherwise delete it in any case.  */

void
record_temp_file (const char *filename, int always_delete, int fail_delete)
{
  char *const name = xstrdup (filename);

  if (always_delete)
    {
      struct temp_file *temp;
      for (temp = always_delete_queue; temp; temp = temp->next)
	if (! filename_cmp (name, temp->name))
	  {
	    free (name);
	    goto already1;
	  }

      temp = XNEW (struct temp_file);
      temp->next = always_delete_queue;
      temp->name = name;
      always_delete_queue = temp;

    already1:;
    }

  if (fail_delete)
    {
      struct temp_file *temp;
      for (temp = failure_delete_queue; temp; temp = temp->next)
	if (! filename_cmp (name, temp->name))
	  {
	    free (name);
	    goto already2;
	  }

      temp = XNEW (struct temp_file);
      temp->next = failure_delete_queue;
      temp->name = name;
      failure_delete_queue = temp;

    already2:;
    }
}

/* Delete all the temporary files whose names we previously recorded.  */

#ifndef DELETE_IF_ORDINARY
#define DELETE_IF_ORDINARY(NAME,ST,VERBOSE_FLAG)        \
do                                                      \
  {                                                     \
    if (stat (NAME, &ST) >= 0 && S_ISREG (ST.st_mode))  \
      if (unlink (NAME) < 0)                            \
	if (VERBOSE_FLAG)                               \
	  error ("%s: %m", (NAME));			\
  } while (0)
#endif

static void
delete_if_ordinary (const char *name)
{
  struct stat st;
#ifdef DEBUG
  int i, c;

  printf ("Delete %s? (y or n) ", name);
  fflush (stdout);
  i = getchar ();
  if (i != '\n')
    while ((c = getchar ()) != '\n' && c != EOF)
      ;

  if (i == 'y' || i == 'Y')
#endif /* DEBUG */
  DELETE_IF_ORDINARY (name, st, verbose_flag);
}

static void
delete_temp_files (void)
{
  struct temp_file *temp;

  for (temp = always_delete_queue; temp; temp = temp->next)
    delete_if_ordinary (temp->name);
  always_delete_queue = 0;
}

/* Delete all the files to be deleted on error.  */

static void
delete_failure_queue (void)
{
  struct temp_file *temp;

  for (temp = failure_delete_queue; temp; temp = temp->next)
    delete_if_ordinary (temp->name);
}

static void
clear_failure_queue (void)
{
  failure_delete_queue = 0;
}

/* Call CALLBACK for each path in PATHS, breaking out early if CALLBACK
   returns non-NULL.
   If DO_MULTI is true iterate over the paths twice, first with multilib
   suffix then without, otherwise iterate over the paths once without
   adding a multilib suffix.  When DO_MULTI is true, some attempt is made
   to avoid visiting the same path twice, but we could do better.  For
   instance, /usr/lib/../lib is considered different from /usr/lib.
   At least EXTRA_SPACE chars past the end of the path passed to
   CALLBACK are available for use by the callback.
   CALLBACK_INFO allows extra parameters to be passed to CALLBACK.

   Returns the value returned by CALLBACK.  */

static void *
for_each_path (const struct path_prefix *paths,
	       bool do_multi,
	       size_t extra_space,
	       void *(*callback) (char *, void *),
	       void *callback_info)
{
  struct prefix_list *pl;
  const char *multi_dir = NULL;
  const char *multi_os_dir = NULL;
  const char *multiarch_suffix = NULL;
  const char *multi_suffix;
  const char *just_multi_suffix;
  char *path = NULL;
  void *ret = NULL;
  bool skip_multi_dir = false;
  bool skip_multi_os_dir = false;

  multi_suffix = machine_suffix;
  just_multi_suffix = just_machine_suffix;
  if (do_multi && multilib_dir && strcmp (multilib_dir, ".") != 0)
    {
      multi_dir = concat (multilib_dir, dir_separator_str, NULL);
      multi_suffix = concat (multi_suffix, multi_dir, NULL);
      just_multi_suffix = concat (just_multi_suffix, multi_dir, NULL);
    }
  if (do_multi && multilib_os_dir && strcmp (multilib_os_dir, ".") != 0)
    multi_os_dir = concat (multilib_os_dir, dir_separator_str, NULL);
  if (multiarch_dir)
    multiarch_suffix = concat (multiarch_dir, dir_separator_str, NULL);

  while (1)
    {
      size_t multi_dir_len = 0;
      size_t multi_os_dir_len = 0;
      size_t multiarch_len = 0;
      size_t suffix_len;
      size_t just_suffix_len;
      size_t len;

      if (multi_dir)
	multi_dir_len = strlen (multi_dir);
      if (multi_os_dir)
	multi_os_dir_len = strlen (multi_os_dir);
      if (multiarch_suffix)
	multiarch_len = strlen (multiarch_suffix);
      suffix_len = strlen (multi_suffix);
      just_suffix_len = strlen (just_multi_suffix);

      if (path == NULL)
	{
	  len = paths->max_len + extra_space + 1;
	  len += MAX (MAX (suffix_len, multi_os_dir_len), multiarch_len);
	  path = XNEWVEC (char, len);
	}

      for (pl = paths->plist; pl != 0; pl = pl->next)
	{
	  len = strlen (pl->prefix);
	  memcpy (path, pl->prefix, len);

	  /* Look first in MACHINE/VERSION subdirectory.  */
	  if (!skip_multi_dir)
	    {
	      memcpy (path + len, multi_suffix, suffix_len + 1);
	      ret = callback (path, callback_info);
	      if (ret)
		break;
	    }

	  /* Some paths are tried with just the machine (ie. target)
	     subdir.  This is used for finding as, ld, etc.  */
	  if (!skip_multi_dir
	      && pl->require_machine_suffix == 2)
	    {
	      memcpy (path + len, just_multi_suffix, just_suffix_len + 1);
	      ret = callback (path, callback_info);
	      if (ret)
		break;
	    }

	  /* Now try the multiarch path.  */
	  if (!skip_multi_dir
	      && !pl->require_machine_suffix && multiarch_dir)
	    {
	      memcpy (path + len, multiarch_suffix, multiarch_len + 1);
	      ret = callback (path, callback_info);
	      if (ret)
		break;
	    }

	  /* Now try the base path.  */
	  if (!pl->require_machine_suffix
	      && !(pl->os_multilib ? skip_multi_os_dir : skip_multi_dir))
	    {
	      const char *this_multi;
	      size_t this_multi_len;

	      if (pl->os_multilib)
		{
		  this_multi = multi_os_dir;
		  this_multi_len = multi_os_dir_len;
		}
	      else
		{
		  this_multi = multi_dir;
		  this_multi_len = multi_dir_len;
		}

	      if (this_multi_len)
		memcpy (path + len, this_multi, this_multi_len + 1);
	      else
		path[len] = '\0';

	      ret = callback (path, callback_info);
	      if (ret)
		break;
	    }
	}
      if (pl)
	break;

      if (multi_dir == NULL && multi_os_dir == NULL)
	break;

      /* Run through the paths again, this time without multilibs.
	 Don't repeat any we have already seen.  */
      if (multi_dir)
	{
	  free (CONST_CAST (char *, multi_dir));
	  multi_dir = NULL;
	  free (CONST_CAST (char *, multi_suffix));
	  multi_suffix = machine_suffix;
	  free (CONST_CAST (char *, just_multi_suffix));
	  just_multi_suffix = just_machine_suffix;
	}
      else
	skip_multi_dir = true;
      if (multi_os_dir)
	{
	  free (CONST_CAST (char *, multi_os_dir));
	  multi_os_dir = NULL;
	}
      else
	skip_multi_os_dir = true;
    }

  if (multi_dir)
    {
      free (CONST_CAST (char *, multi_dir));
      free (CONST_CAST (char *, multi_suffix));
      free (CONST_CAST (char *, just_multi_suffix));
    }
  if (multi_os_dir)
    free (CONST_CAST (char *, multi_os_dir));
  if (ret != path)
    free (path);
  return ret;
}

/* Callback for build_search_list.  Adds path to obstack being built.  */

struct add_to_obstack_info {
  struct obstack *ob;
  bool check_dir;
  bool first_time;
};

static void *
add_to_obstack (char *path, void *data)
{
  struct add_to_obstack_info *info = (struct add_to_obstack_info *) data;

  if (info->check_dir && !is_directory (path, false))
    return NULL;

  if (!info->first_time)
    obstack_1grow (info->ob, PATH_SEPARATOR);

  obstack_grow (info->ob, path, strlen (path));

  info->first_time = false;
  return NULL;
}

/* Add or change the value of an environment variable, outputting the
   change to standard error if in verbose mode.  */
static void
xputenv (const char *string)
{
  env.xput (string);
}

/* Build a list of search directories from PATHS.
   PREFIX is a string to prepend to the list.
   If CHECK_DIR_P is true we ensure the directory exists.
   If DO_MULTI is true, multilib paths are output first, then
   non-multilib paths.
   This is used mostly by putenv_from_prefixes so we use `collect_obstack'.
   It is also used by the --print-search-dirs flag.  */

static char *
build_search_list (const struct path_prefix *paths, const char *prefix,
		   bool check_dir, bool do_multi)
{
  struct add_to_obstack_info info;

  info.ob = &collect_obstack;
  info.check_dir = check_dir;
  info.first_time = true;

  obstack_grow (&collect_obstack, prefix, strlen (prefix));
  obstack_1grow (&collect_obstack, '=');

  for_each_path (paths, do_multi, 0, add_to_obstack, &info);

  obstack_1grow (&collect_obstack, '\0');
  return XOBFINISH (&collect_obstack, char *);
}

/* Rebuild the COMPILER_PATH and LIBRARY_PATH environment variables
   for collect.  */

static void
putenv_from_prefixes (const struct path_prefix *paths, const char *env_var,
		      bool do_multi)
{
  xputenv (build_search_list (paths, env_var, true, do_multi));
}

/* Check whether NAME can be accessed in MODE.  This is like access,
   except that it never considers directories to be executable.  */

static int
access_check (const char *name, int mode)
{
  if (mode == X_OK)
    {
      struct stat st;

      if (stat (name, &st) < 0
	  || S_ISDIR (st.st_mode))
	return -1;
    }

  return access (name, mode);
}

/* Callback for find_a_file.  Appends the file name to the directory
   path.  If the resulting file exists in the right mode, return the
   full pathname to the file.  */

struct file_at_path_info {
  const char *name;
  const char *suffix;
  int name_len;
  int suffix_len;
  int mode;
};

static void *
file_at_path (char *path, void *data)
{
  struct file_at_path_info *info = (struct file_at_path_info *) data;
  size_t len = strlen (path);

  memcpy (path + len, info->name, info->name_len);
  len += info->name_len;

  /* Some systems have a suffix for executable files.
     So try appending that first.  */
  if (info->suffix_len)
    {
      memcpy (path + len, info->suffix, info->suffix_len + 1);
      if (access_check (path, info->mode) == 0)
	return path;
    }

  path[len] = '\0';
  if (access_check (path, info->mode) == 0)
    return path;

  return NULL;
}

/* Search for NAME using the prefix list PREFIXES.  MODE is passed to
   access to check permissions.  If DO_MULTI is true, search multilib
   paths then non-multilib paths, otherwise do not search multilib paths.
   Return 0 if not found, otherwise return its name, allocated with malloc.  */

static char *
find_a_file (const struct path_prefix *pprefix, const char *name, int mode,
	     bool do_multi)
{
  struct file_at_path_info info;

#ifdef DEFAULT_ASSEMBLER
  if (! strcmp (name, "as") && access (DEFAULT_ASSEMBLER, mode) == 0)
    return xstrdup (DEFAULT_ASSEMBLER);
#endif

#ifdef DEFAULT_LINKER
  if (! strcmp (name, "ld") && access (DEFAULT_LINKER, mode) == 0)
    return xstrdup (DEFAULT_LINKER);
#endif

  /* Determine the filename to execute (special case for absolute paths).  */

  if (IS_ABSOLUTE_PATH (name))
    {
      if (access (name, mode) == 0)
	return xstrdup (name);

      return NULL;
    }

  info.name = name;
  info.suffix = (mode & X_OK) != 0 ? HOST_EXECUTABLE_SUFFIX : "";
  info.name_len = strlen (info.name);
  info.suffix_len = strlen (info.suffix);
  info.mode = mode;

  return (char*) for_each_path (pprefix, do_multi,
				info.name_len + info.suffix_len,
				file_at_path, &info);
}

/* Ranking of prefixes in the sort list. -B prefixes are put before
   all others.  */

enum path_prefix_priority
{
  PREFIX_PRIORITY_B_OPT,
  PREFIX_PRIORITY_LAST
};

/* Add an entry for PREFIX in PLIST.  The PLIST is kept in ascending
   order according to PRIORITY.  Within each PRIORITY, new entries are
   appended.

   If WARN is nonzero, we will warn if no file is found
   through this prefix.  WARN should point to an int
   which will be set to 1 if this entry is used.

   COMPONENT is the value to be passed to update_path.

   REQUIRE_MACHINE_SUFFIX is 1 if this prefix can't be used without
   the complete value of machine_suffix.
   2 means try both machine_suffix and just_machine_suffix.  */

static void
add_prefix (struct path_prefix *pprefix, const char *prefix,
	    const char *component, /* enum prefix_priority */ int priority,
	    int require_machine_suffix, int os_multilib)
{
  struct prefix_list *pl, **prev;
  int len;

  for (prev = &pprefix->plist;
       (*prev) != NULL && (*prev)->priority <= priority;
       prev = &(*prev)->next)
    ;

  /* Keep track of the longest prefix.  */

  prefix = update_path (prefix, component);
  len = strlen (prefix);
  if (len > pprefix->max_len)
    pprefix->max_len = len;

  pl = XNEW (struct prefix_list);
  pl->prefix = prefix;
  pl->require_machine_suffix = require_machine_suffix;
  pl->priority = priority;
  pl->os_multilib = os_multilib;

  /* Insert after PREV.  */
  pl->next = (*prev);
  (*prev) = pl;
}

/* Same as add_prefix, but prepending target_system_root to prefix.  */
/* The target_system_root prefix has been relocated by gcc_exec_prefix.  */
static void
add_sysrooted_prefix (struct path_prefix *pprefix, const char *prefix,
		      const char *component,
		      /* enum prefix_priority */ int priority,
		      int require_machine_suffix, int os_multilib)
{
  if (!IS_ABSOLUTE_PATH (prefix))
    fatal_error (input_location, "system path %qs is not absolute", prefix);

  if (target_system_root)
    {
      char *sysroot_no_trailing_dir_separator = xstrdup (target_system_root);
      size_t sysroot_len = strlen (target_system_root);

      if (sysroot_len > 0
	  && target_system_root[sysroot_len - 1] == DIR_SEPARATOR)
	sysroot_no_trailing_dir_separator[sysroot_len - 1] = '\0';

      if (target_sysroot_suffix)
	prefix = concat (sysroot_no_trailing_dir_separator,
			 target_sysroot_suffix, prefix, NULL);
      else
	prefix = concat (sysroot_no_trailing_dir_separator, prefix, NULL);

      free (sysroot_no_trailing_dir_separator);

      /* We have to override this because GCC's notion of sysroot
	 moves along with GCC.  */
      component = "GCC";
    }

  add_prefix (pprefix, prefix, component, priority,
	      require_machine_suffix, os_multilib);
}

/* Same as add_prefix, but prepending target_sysroot_hdrs_suffix to prefix.  */

static void
add_sysrooted_hdrs_prefix (struct path_prefix *pprefix, const char *prefix,
			   const char *component,
			   /* enum prefix_priority */ int priority,
			   int require_machine_suffix, int os_multilib)
{
  if (!IS_ABSOLUTE_PATH (prefix))
    fatal_error (input_location, "system path %qs is not absolute", prefix);

  if (target_system_root)
    {
      char *sysroot_no_trailing_dir_separator = xstrdup (target_system_root);
      size_t sysroot_len = strlen (target_system_root);

      if (sysroot_len > 0
	  && target_system_root[sysroot_len - 1] == DIR_SEPARATOR)
	sysroot_no_trailing_dir_separator[sysroot_len - 1] = '\0';

      if (target_sysroot_hdrs_suffix)
	prefix = concat (sysroot_no_trailing_dir_separator,
			 target_sysroot_hdrs_suffix, prefix, NULL);
      else
	prefix = concat (sysroot_no_trailing_dir_separator, prefix, NULL);

      free (sysroot_no_trailing_dir_separator);

      /* We have to override this because GCC's notion of sysroot
	 moves along with GCC.  */
      component = "GCC";
    }

  add_prefix (pprefix, prefix, component, priority,
	      require_machine_suffix, os_multilib);
}


/* Execute the command specified by the arguments on the current line of spec.
   When using pipes, this includes several piped-together commands
   with `|' between them.

   Return 0 if successful, -1 if failed.  */

static int
execute (void)
{
  int i;
  int n_commands;		/* # of command.  */
  char *string;
  struct pex_obj *pex;
  struct command
  {
    const char *prog;		/* program name.  */
    const char **argv;		/* vector of args.  */
  };
  const char *arg;

  struct command *commands;	/* each command buffer with above info.  */

  gcc_assert (!processing_spec_function);

  if (wrapper_string)
    {
      string = find_a_file (&exec_prefixes,
			    argbuf[0], X_OK, false);
      if (string)
	argbuf[0] = string;
      insert_wrapper (wrapper_string);
    }

  /* Count # of piped commands.  */
  for (n_commands = 1, i = 0; argbuf.iterate (i, &arg); i++)
    if (strcmp (arg, "|") == 0)
      n_commands++;

  /* Get storage for each command.  */
  commands = (struct command *) alloca (n_commands * sizeof (struct command));

  /* Split argbuf into its separate piped processes,
     and record info about each one.
     Also search for the programs that are to be run.  */

  argbuf.safe_push (0);

  commands[0].prog = argbuf[0]; /* first command.  */
  commands[0].argv = argbuf.address ();

  if (!wrapper_string)
    {
      string = find_a_file (&exec_prefixes, commands[0].prog, X_OK, false);
      if (string)
	commands[0].argv[0] = string;
    }

  for (n_commands = 1, i = 0; argbuf.iterate (i, &arg); i++)
    if (arg && strcmp (arg, "|") == 0)
      {				/* each command.  */
#if defined (__MSDOS__) || defined (OS2) || defined (VMS)
	fatal_error (input_location, "%<-pipe%> not supported");
#endif
	argbuf[i] = 0; /* Termination of command args.  */
	commands[n_commands].prog = argbuf[i + 1];
	commands[n_commands].argv
	  = &(argbuf.address ())[i + 1];
	string = find_a_file (&exec_prefixes, commands[n_commands].prog,
			      X_OK, false);
	if (string)
	  commands[n_commands].argv[0] = string;
	n_commands++;
      }

  /* If -v, print what we are about to do, and maybe query.  */

  if (verbose_flag)
    {
      /* For help listings, put a blank line between sub-processes.  */
      if (print_help_list)
	fputc ('\n', stderr);

      /* Print each piped command as a separate line.  */
      for (i = 0; i < n_commands; i++)
	{
	  const char *const *j;

	  if (verbose_only_flag)
	    {
	      for (j = commands[i].argv; *j; j++)
		{
		  const char *p;
		  for (p = *j; *p; ++p)
		    if (!ISALNUM ((unsigned char) *p)
			&& *p != '_' && *p != '/' && *p != '-' && *p != '.')
		      break;
		  if (*p || !*j)
		    {
		      fprintf (stderr, " \"");
		      for (p = *j; *p; ++p)
			{
			  if (*p == '"' || *p == '\\' || *p == '$')
			    fputc ('\\', stderr);
			  fputc (*p, stderr);
			}
		      fputc ('"', stderr);
		    }
		  /* If it's empty, print "".  */
		  else if (!**j)
		    fprintf (stderr, " \"\"");
		  else
		    fprintf (stderr, " %s", *j);
		}
	    }
	  else
	    for (j = commands[i].argv; *j; j++)
	      /* If it's empty, print "".  */
	      if (!**j)
		fprintf (stderr, " \"\"");
	      else
		fprintf (stderr, " %s", *j);

	  /* Print a pipe symbol after all but the last command.  */
	  if (i + 1 != n_commands)
	    fprintf (stderr, " |");
	  fprintf (stderr, "\n");
	}
      fflush (stderr);
      if (verbose_only_flag != 0)
        {
	  /* verbose_only_flag should act as if the spec was
	     executed, so increment execution_count before
	     returning.  This prevents spurious warnings about
	     unused linker input files, etc.  */
	  execution_count++;
	  return 0;
        }
#ifdef DEBUG
      fnotice (stderr, "\nGo ahead? (y or n) ");
      fflush (stderr);
      i = getchar ();
      if (i != '\n')
	while (getchar () != '\n')
	  ;

      if (i != 'y' && i != 'Y')
	return 0;
#endif /* DEBUG */
    }

#ifdef ENABLE_VALGRIND_CHECKING
  /* Run the each command through valgrind.  To simplify prepending the
     path to valgrind and the option "-q" (for quiet operation unless
     something triggers), we allocate a separate argv array.  */

  for (i = 0; i < n_commands; i++)
    {
      const char **argv;
      int argc;
      int j;

      for (argc = 0; commands[i].argv[argc] != NULL; argc++)
	;

      argv = XALLOCAVEC (const char *, argc + 3);

      argv[0] = VALGRIND_PATH;
      argv[1] = "-q";
      for (j = 2; j < argc + 2; j++)
	argv[j] = commands[i].argv[j - 2];
      argv[j] = NULL;

      commands[i].argv = argv;
      commands[i].prog = argv[0];
    }
#endif

  /* Run each piped subprocess.  */

  pex = pex_init (PEX_USE_PIPES | ((report_times || report_times_to_file)
				   ? PEX_RECORD_TIMES : 0),
		  progname, temp_filename);
  if (pex == NULL)
    fatal_error (input_location, "%<pex_init%> failed: %m");

  for (i = 0; i < n_commands; i++)
    {
      const char *errmsg;
      int err;
      const char *string = commands[i].argv[0];

      errmsg = pex_run (pex,
			((i + 1 == n_commands ? PEX_LAST : 0)
			 | (string == commands[i].prog ? PEX_SEARCH : 0)),
			string, CONST_CAST (char **, commands[i].argv),
			NULL, NULL, &err);
      if (errmsg != NULL)
	{
	  errno = err;
	  fatal_error (input_location,
		       err ? G_("cannot execute %qs: %s: %m")
		       : G_("cannot execute %qs: %s"),
		       string, errmsg);
	}

      if (i && string != commands[i].prog)
	free (CONST_CAST (char *, string));
    }

  execution_count++;

  /* Wait for all the subprocesses to finish.  */

  {
    int *statuses;
    struct pex_time *times = NULL;
    int ret_code = 0;

    statuses = (int *) alloca (n_commands * sizeof (int));
    if (!pex_get_status (pex, n_commands, statuses))
      fatal_error (input_location, "failed to get exit status: %m");

    if (report_times || report_times_to_file)
      {
	times = (struct pex_time *) alloca (n_commands * sizeof (struct pex_time));
	if (!pex_get_times (pex, n_commands, times))
	  fatal_error (input_location, "failed to get process times: %m");
      }

    pex_free (pex);

    for (i = 0; i < n_commands; ++i)
      {
	int status = statuses[i];

	if (WIFSIGNALED (status))
	  switch (WTERMSIG (status))
	    {
	    case SIGINT:
	    case SIGTERM:
	      /* SIGQUIT and SIGKILL are not available on MinGW.  */
#ifdef SIGQUIT
	    case SIGQUIT:
#endif
#ifdef SIGKILL
	    case SIGKILL:
#endif
	      /* The user (or environment) did something to the
		 inferior.  Making this an ICE confuses the user into
		 thinking there's a compiler bug.  Much more likely is
		 the user or OOM killer nuked it.  */
	      fatal_error (input_location,
			   "%s signal terminated program %s",
			   strsignal (WTERMSIG (status)),
			   commands[i].prog);
	      break;

#ifdef SIGPIPE
	    case SIGPIPE:
	      /* SIGPIPE is a special case.  It happens in -pipe mode
		 when the compiler dies before the preprocessor is
		 done, or the assembler dies before the compiler is
		 done.  There's generally been an error already, and
		 this is just fallout.  So don't generate another
		 error unless we would otherwise have succeeded.  */
	      if (signal_count || greatest_status >= MIN_FATAL_STATUS)
		{
		  signal_count++;
		  ret_code = -1;
		  break;
		}
#endif
	      /* FALLTHROUGH */

	    default:
	      /* The inferior failed to catch the signal.  */
	      internal_error_no_backtrace ("%s signal terminated program %s",
					   strsignal (WTERMSIG (status)),
					   commands[i].prog);
	    }
	else if (WIFEXITED (status)
		 && WEXITSTATUS (status) >= MIN_FATAL_STATUS)
	  {
	    /* For ICEs in cc1, cc1obj, cc1plus see if it is
	       reproducible or not.  */
	    const char *p;
	    if (flag_report_bug
		&& WEXITSTATUS (status) == ICE_EXIT_CODE
		&& i == 0
		&& (p = strrchr (commands[0].argv[0], DIR_SEPARATOR))
		&& ! strncmp (p + 1, "cc1", 3))
	      try_generate_repro (commands[0].argv);
	    if (WEXITSTATUS (status) > greatest_status)
	      greatest_status = WEXITSTATUS (status);
	    ret_code = -1;
	  }

	if (report_times || report_times_to_file)
	  {
	    struct pex_time *pt = &times[i];
	    double ut, st;

	    ut = ((double) pt->user_seconds
		  + (double) pt->user_microseconds / 1.0e6);
	    st = ((double) pt->system_seconds
		  + (double) pt->system_microseconds / 1.0e6);

	    if (ut + st != 0)
	      {
		if (report_times)
		  fnotice (stderr, "# %s %.2f %.2f\n",
			   commands[i].prog, ut, st);

		if (report_times_to_file)
		  {
		    int c = 0;
		    const char *const *j;

		    fprintf (report_times_to_file, "%g %g", ut, st);

		    for (j = &commands[i].prog; *j; j = &commands[i].argv[++c])
		      {
			const char *p;
			for (p = *j; *p; ++p)
			  if (*p == '"' || *p == '\\' || *p == '$'
			      || ISSPACE (*p))
			    break;

			if (*p)
			  {
			    fprintf (report_times_to_file, " \"");
			    for (p = *j; *p; ++p)
			      {
				if (*p == '"' || *p == '\\' || *p == '$')
				  fputc ('\\', report_times_to_file);
				fputc (*p, report_times_to_file);
			      }
			    fputc ('"', report_times_to_file);
			  }
			else
			  fprintf (report_times_to_file, " %s", *j);
		      }

		    fputc ('\n', report_times_to_file);
		  }
	      }
	  }
      }

   if (commands[0].argv[0] != commands[0].prog)
     free (CONST_CAST (char *, commands[0].argv[0]));

    return ret_code;
  }
}

/* Find all the switches given to us
   and make a vector describing them.
   The elements of the vector are strings, one per switch given.
   If a switch uses following arguments, then the `part1' field
   is the switch itself and the `args' field
   is a null-terminated vector containing the following arguments.
   Bits in the `live_cond' field are:
   SWITCH_LIVE to indicate this switch is true in a conditional spec.
   SWITCH_FALSE to indicate this switch is overridden by a later switch.
   SWITCH_IGNORE to indicate this switch should be ignored (used in %<S).
   SWITCH_IGNORE_PERMANENTLY to indicate this switch should be ignored.
   SWITCH_KEEP_FOR_GCC to indicate that this switch, otherwise ignored,
   should be included in COLLECT_GCC_OPTIONS.
   in all do_spec calls afterwards.  Used for %<S from self specs.
   The `known' field describes whether this is an internal switch.
   The `validated' field describes whether any spec has looked at this switch;
   if it remains false at the end of the run, the switch must be meaningless.
   The `ordering' field is used to temporarily mark switches that have to be
   kept in a specific order.  */

#define SWITCH_LIVE    			(1 << 0)
#define SWITCH_FALSE   			(1 << 1)
#define SWITCH_IGNORE			(1 << 2)
#define SWITCH_IGNORE_PERMANENTLY	(1 << 3)
#define SWITCH_KEEP_FOR_GCC		(1 << 4)

struct switchstr
{
  const char *part1;
  const char **args;
  unsigned int live_cond;
  bool known;
  bool validated;
  bool ordering;
};

static struct switchstr *switches;

static int n_switches;

static int n_switches_alloc;

/* Set to zero if -fcompare-debug is disabled, positive if it's
   enabled and we're running the first compilation, negative if it's
   enabled and we're running the second compilation.  For most of the
   time, it's in the range -1..1, but it can be temporarily set to 2
   or 3 to indicate that the -fcompare-debug flags didn't come from
   the command-line, but rather from the GCC_COMPARE_DEBUG environment
   variable, until a synthesized -fcompare-debug flag is added to the
   command line.  */
int compare_debug;

/* Set to nonzero if we've seen the -fcompare-debug-second flag.  */
int compare_debug_second;

/* Set to the flags that should be passed to the second compilation in
   a -fcompare-debug compilation.  */
const char *compare_debug_opt;

static struct switchstr *switches_debug_check[2];

static int n_switches_debug_check[2];

static int n_switches_alloc_debug_check[2];

static char *debug_check_temp_file[2];

/* Language is one of three things:

   1) The name of a real programming language.
   2) NULL, indicating that no one has figured out
   what it is yet.
   3) '*', indicating that the file should be passed
   to the linker.  */
struct infile
{
  const char *name;
  const char *language;
  struct compiler *incompiler;
  bool compiled;
  bool preprocessed;
};

/* Also a vector of input files specified.  */

static struct infile *infiles;

int n_infiles;

static int n_infiles_alloc;

/* True if undefined environment variables encountered during spec processing
   are ok to ignore, typically when we're running for --help or --version.  */

static bool spec_undefvar_allowed;

/* True if multiple input files are being compiled to a single
   assembly file.  */

static bool combine_inputs;

/* This counts the number of libraries added by lang_specific_driver, so that
   we can tell if there were any user supplied any files or libraries.  */

static int added_libraries;

/* And a vector of corresponding output files is made up later.  */

const char **outfiles;

#if defined(HAVE_TARGET_OBJECT_SUFFIX) || defined(HAVE_TARGET_EXECUTABLE_SUFFIX)

/* Convert NAME to a new name if it is the standard suffix.  DO_EXE
   is true if we should look for an executable suffix.  DO_OBJ
   is true if we should look for an object suffix.  */

static const char *
convert_filename (const char *name, int do_exe ATTRIBUTE_UNUSED,
		  int do_obj ATTRIBUTE_UNUSED)
{
#if defined(HAVE_TARGET_EXECUTABLE_SUFFIX)
  int i;
#endif
  int len;

  if (name == NULL)
    return NULL;

  len = strlen (name);

#ifdef HAVE_TARGET_OBJECT_SUFFIX
  /* Convert x.o to x.obj if TARGET_OBJECT_SUFFIX is ".obj".  */
  if (do_obj && len > 2
      && name[len - 2] == '.'
      && name[len - 1] == 'o')
    {
      obstack_grow (&obstack, name, len - 2);
      obstack_grow0 (&obstack, TARGET_OBJECT_SUFFIX, strlen (TARGET_OBJECT_SUFFIX));
      name = XOBFINISH (&obstack, const char *);
    }
#endif

#if defined(HAVE_TARGET_EXECUTABLE_SUFFIX)
  /* If there is no filetype, make it the executable suffix (which includes
     the ".").  But don't get confused if we have just "-o".  */
  if (! do_exe || TARGET_EXECUTABLE_SUFFIX[0] == 0 || (len == 2 && name[0] == '-'))
    return name;

  for (i = len - 1; i >= 0; i--)
    if (IS_DIR_SEPARATOR (name[i]))
      break;

  for (i++; i < len; i++)
    if (name[i] == '.')
      return name;

  obstack_grow (&obstack, name, len);
  obstack_grow0 (&obstack, TARGET_EXECUTABLE_SUFFIX,
		 strlen (TARGET_EXECUTABLE_SUFFIX));
  name = XOBFINISH (&obstack, const char *);
#endif

  return name;
}
#endif

/* Display the command line switches accepted by gcc.  */
static void
display_help (void)
{
  printf (_("Usage: %s [options] file...\n"), progname);
  fputs (_("Options:\n"), stdout);

  fputs (_("  -pass-exit-codes         Exit with highest error code from a phase.\n"), stdout);
  fputs (_("  --help                   Display this information.\n"), stdout);
  fputs (_("  --target-help            Display target specific command line options.\n"), stdout);
  fputs (_("  --help={common|optimizers|params|target|warnings|[^]{joined|separate|undocumented}}[,...].\n"), stdout);
  fputs (_("                           Display specific types of command line options.\n"), stdout);
  if (! verbose_flag)
    fputs (_("  (Use '-v --help' to display command line options of sub-processes).\n"), stdout);
  fputs (_("  --version                Display compiler version information.\n"), stdout);
  fputs (_("  -dumpspecs               Display all of the built in spec strings.\n"), stdout);
  fputs (_("  -dumpversion             Display the version of the compiler.\n"), stdout);
  fputs (_("  -dumpmachine             Display the compiler's target processor.\n"), stdout);
  fputs (_("  -print-search-dirs       Display the directories in the compiler's search path.\n"), stdout);
  fputs (_("  -print-libgcc-file-name  Display the name of the compiler's companion library.\n"), stdout);
  fputs (_("  -print-file-name=<lib>   Display the full path to library <lib>.\n"), stdout);
  fputs (_("  -print-prog-name=<prog>  Display the full path to compiler component <prog>.\n"), stdout);
  fputs (_("\
  -print-multiarch         Display the target's normalized GNU triplet, used as\n\
                           a component in the library path.\n"), stdout);
  fputs (_("  -print-multi-directory   Display the root directory for versions of libgcc.\n"), stdout);
  fputs (_("\
  -print-multi-lib         Display the mapping between command line options and\n\
                           multiple library search directories.\n"), stdout);
  fputs (_("  -print-multi-os-directory Display the relative path to OS libraries.\n"), stdout);
  fputs (_("  -print-sysroot           Display the target libraries directory.\n"), stdout);
  fputs (_("  -print-sysroot-headers-suffix Display the sysroot suffix used to find headers.\n"), stdout);
  fputs (_("  -Wa,<options>            Pass comma-separated <options> on to the assembler.\n"), stdout);
  fputs (_("  -Wp,<options>            Pass comma-separated <options> on to the preprocessor.\n"), stdout);
  fputs (_("  -Wl,<options>            Pass comma-separated <options> on to the linker.\n"), stdout);
  fputs (_("  -Xassembler <arg>        Pass <arg> on to the assembler.\n"), stdout);
  fputs (_("  -Xpreprocessor <arg>     Pass <arg> on to the preprocessor.\n"), stdout);
  fputs (_("  -Xlinker <arg>           Pass <arg> on to the linker.\n"), stdout);
  fputs (_("  -save-temps              Do not delete intermediate files.\n"), stdout);
  fputs (_("  -save-temps=<arg>        Do not delete intermediate files.\n"), stdout);
  fputs (_("\
  -no-canonical-prefixes   Do not canonicalize paths when building relative\n\
                           prefixes to other gcc components.\n"), stdout);
  fputs (_("  -pipe                    Use pipes rather than intermediate files.\n"), stdout);
  fputs (_("  -time                    Time the execution of each subprocess.\n"), stdout);
  fputs (_("  -specs=<file>            Override built-in specs with the contents of <file>.\n"), stdout);
  fputs (_("  -std=<standard>          Assume that the input sources are for <standard>.\n"), stdout);
  fputs (_("\
  --sysroot=<directory>    Use <directory> as the root directory for headers\n\
                           and libraries.\n"), stdout);
  fputs (_("  -B <directory>           Add <directory> to the compiler's search paths.\n"), stdout);
  fputs (_("  -v                       Display the programs invoked by the compiler.\n"), stdout);
  fputs (_("  -###                     Like -v but options quoted and commands not executed.\n"), stdout);
  fputs (_("  -E                       Preprocess only; do not compile, assemble or link.\n"), stdout);
  fputs (_("  -S                       Compile only; do not assemble or link.\n"), stdout);
  fputs (_("  -c                       Compile and assemble, but do not link.\n"), stdout);
  fputs (_("  -o <file>                Place the output into <file>.\n"), stdout);
  fputs (_("  -pie                     Create a dynamically linked position independent\n\
                           executable.\n"), stdout);
  fputs (_("  -shared                  Create a shared library.\n"), stdout);
  fputs (_("\
  -x <language>            Specify the language of the following input files.\n\
                           Permissible languages include: c c++ assembler none\n\
                           'none' means revert to the default behavior of\n\
                           guessing the language based on the file's extension.\n\
"), stdout);

  printf (_("\
\nOptions starting with -g, -f, -m, -O, -W, or --param are automatically\n\
 passed on to the various sub-processes invoked by %s.  In order to pass\n\
 other options on to these processes the -W<letter> options must be used.\n\
"), progname);

  /* The rest of the options are displayed by invocations of the various
     sub-processes.  */
}

static void
add_preprocessor_option (const char *option, int len)
{
  preprocessor_options.safe_push (save_string (option, len));
}

static void
add_assembler_option (const char *option, int len)
{
  assembler_options.safe_push (save_string (option, len));
}

static void
add_linker_option (const char *option, int len)
{
  linker_options.safe_push (save_string (option, len));
}

/* Allocate space for an input file in infiles.  */

static void
alloc_infile (void)
{
  if (n_infiles_alloc == 0)
    {
      n_infiles_alloc = 16;
      infiles = XNEWVEC (struct infile, n_infiles_alloc);
    }
  else if (n_infiles_alloc == n_infiles)
    {
      n_infiles_alloc *= 2;
      infiles = XRESIZEVEC (struct infile, infiles, n_infiles_alloc);
    }
}

/* Store an input file with the given NAME and LANGUAGE in
   infiles.  */

static void
add_infile (const char *name, const char *language)
{
  alloc_infile ();
  infiles[n_infiles].name = name;
  infiles[n_infiles++].language = language;
}

/* Allocate space for a switch in switches.  */

static void
alloc_switch (void)
{
  if (n_switches_alloc == 0)
    {
      n_switches_alloc = 16;
      switches = XNEWVEC (struct switchstr, n_switches_alloc);
    }
  else if (n_switches_alloc == n_switches)
    {
      n_switches_alloc *= 2;
      switches = XRESIZEVEC (struct switchstr, switches, n_switches_alloc);
    }
}

/* Save an option OPT with N_ARGS arguments in array ARGS, marking it
   as validated if VALIDATED and KNOWN if it is an internal switch.  */

static void
save_switch (const char *opt, size_t n_args, const char *const *args,
	     bool validated, bool known)
{
  alloc_switch ();
  switches[n_switches].part1 = opt + 1;
  if (n_args == 0)
    switches[n_switches].args = 0;
  else
    {
      switches[n_switches].args = XNEWVEC (const char *, n_args + 1);
      memcpy (switches[n_switches].args, args, n_args * sizeof (const char *));
      switches[n_switches].args[n_args] = NULL;
    }

  switches[n_switches].live_cond = 0;
  switches[n_switches].validated = validated;
  switches[n_switches].known = known;
  switches[n_switches].ordering = 0;
  n_switches++;
}

/* Set the SOURCE_DATE_EPOCH environment variable to the current time if it is
   not set already.  */

static void
set_source_date_epoch_envvar ()
{
  /* Array size is 21 = ceil(log_10(2^64)) + 1 to hold string representations
     of 64 bit integers.  */
  char source_date_epoch[21];
  time_t tt;

  errno = 0;
  tt = time (NULL);
  if (tt < (time_t) 0 || errno != 0)
    tt = (time_t) 0;

  snprintf (source_date_epoch, 21, "%llu", (unsigned long long) tt);
  /* Using setenv instead of xputenv because we want the variable to remain
     after finalizing so that it's still set in the second run when using
     -fcompare-debug.  */
  setenv ("SOURCE_DATE_EPOCH", source_date_epoch, 0);
}

/* Handle an option DECODED that is unknown to the option-processing
   machinery.  */

static bool
driver_unknown_option_callback (const struct cl_decoded_option *decoded)
{
  const char *opt = decoded->arg;
  if (opt[1] == 'W' && opt[2] == 'n' && opt[3] == 'o' && opt[4] == '-'
      && !(decoded->errors & CL_ERR_NEGATIVE))
    {
      /* Leave unknown -Wno-* options for the compiler proper, to be
	 diagnosed only if there are warnings.  */
      save_switch (decoded->canonical_option[0],
		   decoded->canonical_option_num_elements - 1,
		   &decoded->canonical_option[1], false, true);
      return false;
    }
  if (decoded->opt_index == OPT_SPECIAL_unknown)
    {
      /* Give it a chance to define it a spec file.  */
      save_switch (decoded->canonical_option[0],
		   decoded->canonical_option_num_elements - 1,
		   &decoded->canonical_option[1], false, false);
      return false;
    }
  else
    return true;
}

/* Handle an option DECODED that is not marked as CL_DRIVER.
   LANG_MASK will always be CL_DRIVER.  */

static void
driver_wrong_lang_callback (const struct cl_decoded_option *decoded,
			    unsigned int lang_mask ATTRIBUTE_UNUSED)
{
  /* At this point, non-driver options are accepted (and expected to
     be passed down by specs) unless marked to be rejected by the
     driver.  Options to be rejected by the driver but accepted by the
     compilers proper are treated just like completely unknown
     options.  */
  const struct cl_option *option = &cl_options[decoded->opt_index];

  if (option->cl_reject_driver)
    error ("unrecognized command-line option %qs",
	   decoded->orig_option_with_args_text);
  else
    save_switch (decoded->canonical_option[0],
		 decoded->canonical_option_num_elements - 1,
		 &decoded->canonical_option[1], false, true);
}

static const char *spec_lang = 0;
static int last_language_n_infiles;

/* Parse -foffload option argument.  */

static void
handle_foffload_option (const char *arg)
{
  const char *c, *cur, *n, *next, *end;
  char *target;

  /* If option argument starts with '-' then no target is specified and we
     do not need to parse it.  */
  if (arg[0] == '-')
    return;

  end = strchr (arg, '=');
  if (end == NULL)
    end = strchr (arg, '\0');
  cur = arg;

  while (cur < end)
    {
      next = strchr (cur, ',');
      if (next == NULL)
	next = end;
      next = (next > end) ? end : next;

      target = XNEWVEC (char, next - cur + 1);
      memcpy (target, cur, next - cur);
      target[next - cur] = '\0';

      /* If 'disable' is passed to the option, stop parsing the option and clean
         the list of offload targets.  */
      if (strcmp (target, "disable") == 0)
	{
	  free (offload_targets);
	  offload_targets = xstrdup ("");
	  break;
	}

      /* Check that GCC is configured to support the offload target.  */
      c = OFFLOAD_TARGETS;
      while (c)
	{
	  n = strchr (c, ',');
	  if (n == NULL)
	    n = strchr (c, '\0');

	  if (next - cur == n - c && strncmp (target, c, n - c) == 0)
	    break;

	  c = *n ? n + 1 : NULL;
	}

      if (!c)
	fatal_error (input_location,
		     "GCC is not configured to support %s as offload target",
		     target);

      if (!offload_targets)
	{
	  offload_targets = target;
	  target = NULL;
	}
      else
	{
	  /* Check that the target hasn't already presented in the list.  */
	  c = offload_targets;
	  do
	    {
	      n = strchr (c, ':');
	      if (n == NULL)
		n = strchr (c, '\0');

	      if (next - cur == n - c && strncmp (c, target, n - c) == 0)
		break;

	      c = n + 1;
	    }
	  while (*n);

	  /* If duplicate is not found, append the target to the list.  */
	  if (c > n)
	    {
	      size_t offload_targets_len = strlen (offload_targets);
	      offload_targets
		= XRESIZEVEC (char, offload_targets,
			      offload_targets_len + 1 + next - cur + 1);
	      offload_targets[offload_targets_len++] = ':';
	      memcpy (offload_targets + offload_targets_len, target, next - cur + 1);
	    }
	}

      cur = next + 1;
      XDELETEVEC (target);
    }
}

/* Handle a driver option; arguments and return value as for
   handle_option.  */

static bool
driver_handle_option (struct gcc_options *opts,
		      struct gcc_options *opts_set,
		      const struct cl_decoded_option *decoded,
		      unsigned int lang_mask ATTRIBUTE_UNUSED, int kind,
		      location_t loc,
		      const struct cl_option_handlers *handlers ATTRIBUTE_UNUSED,
		      diagnostic_context *dc,
		      void (*) (void))
{
  size_t opt_index = decoded->opt_index;
  const char *arg = decoded->arg;
  const char *compare_debug_replacement_opt;
  int value = decoded->value;
  bool validated = false;
  bool do_save = true;

  gcc_assert (opts == &global_options);
  gcc_assert (opts_set == &global_options_set);
  gcc_assert (kind == DK_UNSPECIFIED);
  gcc_assert (loc == UNKNOWN_LOCATION);
  gcc_assert (dc == global_dc);

  switch (opt_index)
    {
    case OPT_dumpspecs:
      {
	struct spec_list *sl;
	init_spec ();
	for (sl = specs; sl; sl = sl->next)
	  printf ("*%s:\n%s\n\n", sl->name, *(sl->ptr_spec));
	if (link_command_spec)
	  printf ("*link_command:\n%s\n\n", link_command_spec);
	exit (0);
      }

    case OPT_dumpversion:
      printf ("%s\n", spec_version);
      exit (0);

    case OPT_dumpmachine:
      printf ("%s\n", spec_machine);
      exit (0);

    case OPT_dumpfullversion:
      printf ("%s\n", BASEVER);
      exit (0);

    case OPT__version:
      print_version = 1;

      /* CPP driver cannot obtain switch from cc1_options.  */
      if (is_cpp_driver)
	add_preprocessor_option ("--version", strlen ("--version"));
      add_assembler_option ("--version", strlen ("--version"));
      add_linker_option ("--version", strlen ("--version"));
      break;

    case OPT__completion_:
      validated = true;
      completion = decoded->arg;
      break;

    case OPT__help:
      print_help_list = 1;

      /* CPP driver cannot obtain switch from cc1_options.  */
      if (is_cpp_driver)
	add_preprocessor_option ("--help", 6);
      add_assembler_option ("--help", 6);
      add_linker_option ("--help", 6);
      break;

    case OPT__help_:
      print_subprocess_help = 2;
      break;

    case OPT__target_help:
      print_subprocess_help = 1;

      /* CPP driver cannot obtain switch from cc1_options.  */
      if (is_cpp_driver)
	add_preprocessor_option ("--target-help", 13);
      add_assembler_option ("--target-help", 13);
      add_linker_option ("--target-help", 13);
      break;

    case OPT__no_sysroot_suffix:
    case OPT_pass_exit_codes:
    case OPT_print_search_dirs:
    case OPT_print_file_name_:
    case OPT_print_prog_name_:
    case OPT_print_multi_lib:
    case OPT_print_multi_directory:
    case OPT_print_sysroot:
    case OPT_print_multi_os_directory:
    case OPT_print_multiarch:
    case OPT_print_sysroot_headers_suffix:
    case OPT_time:
    case OPT_wrapper:
      /* These options set the variables specified in common.opt
	 automatically, and do not need to be saved for spec
	 processing.  */
      do_save = false;
      break;

    case OPT_print_libgcc_file_name:
      print_file_name = "libgcc.a";
      do_save = false;
      break;

    case OPT_fuse_ld_bfd:
       use_ld = ".bfd";
       break;

    case OPT_fuse_ld_gold:
       use_ld = ".gold";
       break;

    case OPT_fcompare_debug_second:
      compare_debug_second = 1;
      break;

    case OPT_fcompare_debug:
      switch (value)
	{
	case 0:
	  compare_debug_replacement_opt = "-fcompare-debug=";
	  arg = "";
	  goto compare_debug_with_arg;

	case 1:
	  compare_debug_replacement_opt = "-fcompare-debug=-gtoggle";
	  arg = "-gtoggle";
	  goto compare_debug_with_arg;

	default:
	  gcc_unreachable ();
	}
      break;

    case OPT_fcompare_debug_:
      compare_debug_replacement_opt = decoded->canonical_option[0];
    compare_debug_with_arg:
      gcc_assert (decoded->canonical_option_num_elements == 1);
      gcc_assert (arg != NULL);
      if (*arg)
	compare_debug = 1;
      else
	compare_debug = -1;
      if (compare_debug < 0)
	compare_debug_opt = NULL;
      else
	compare_debug_opt = arg;
      save_switch (compare_debug_replacement_opt, 0, NULL, validated, true);
      set_source_date_epoch_envvar ();
      return true;

    case OPT_fdiagnostics_color_:
      diagnostic_color_init (dc, value);
      break;

    case OPT_fdiagnostics_urls_:
      diagnostic_urls_init (dc, value);
      break;

    case OPT_fdiagnostics_format_:
      diagnostic_output_format_init (dc,
				     (enum diagnostics_output_format)value);
      break;

    case OPT_Wa_:
      {
	int prev, j;
	/* Pass the rest of this option to the assembler.  */

	/* Split the argument at commas.  */
	prev = 0;
	for (j = 0; arg[j]; j++)
	  if (arg[j] == ',')
	    {
	      add_assembler_option (arg + prev, j - prev);
	      prev = j + 1;
	    }

	/* Record the part after the last comma.  */
	add_assembler_option (arg + prev, j - prev);
      }
      do_save = false;
      break;

    case OPT_Wp_:
      {
	int prev, j;
	/* Pass the rest of this option to the preprocessor.  */

	/* Split the argument at commas.  */
	prev = 0;
	for (j = 0; arg[j]; j++)
	  if (arg[j] == ',')
	    {
	      add_preprocessor_option (arg + prev, j - prev);
	      prev = j + 1;
	    }

	/* Record the part after the last comma.  */
	add_preprocessor_option (arg + prev, j - prev);
      }
      do_save = false;
      break;

    case OPT_Wl_:
      {
	int prev, j;
	/* Split the argument at commas.  */
	prev = 0;
	for (j = 0; arg[j]; j++)
	  if (arg[j] == ',')
	    {
	      add_infile (save_string (arg + prev, j - prev), "*");
	      prev = j + 1;
	    }
	/* Record the part after the last comma.  */
	add_infile (arg + prev, "*");
      }
      do_save = false;
      break;

    case OPT_Xlinker:
      add_infile (arg, "*");
      do_save = false;
      break;

    case OPT_Xpreprocessor:
      add_preprocessor_option (arg, strlen (arg));
      do_save = false;
      break;

    case OPT_Xassembler:
      add_assembler_option (arg, strlen (arg));
      do_save = false;
      break;

    case OPT_l:
      /* POSIX allows separation of -l and the lib arg; canonicalize
	 by concatenating -l with its arg */
      add_infile (concat ("-l", arg, NULL), "*");
      do_save = false;
      break;

    case OPT_L:
      /* Similarly, canonicalize -L for linkers that may not accept
	 separate arguments.  */
      save_switch (concat ("-L", arg, NULL), 0, NULL, validated, true);
      return true;

    case OPT_F:
      /* Likewise -F.  */
      save_switch (concat ("-F", arg, NULL), 0, NULL, validated, true);
      return true;

    case OPT_save_temps:
      save_temps_flag = SAVE_TEMPS_CWD;
      validated = true;
      break;

    case OPT_save_temps_:
      if (strcmp (arg, "cwd") == 0)
	save_temps_flag = SAVE_TEMPS_CWD;
      else if (strcmp (arg, "obj") == 0
	       || strcmp (arg, "object") == 0)
	save_temps_flag = SAVE_TEMPS_OBJ;
      else
	fatal_error (input_location, "%qs is an unknown %<-save-temps%> option",
		     decoded->orig_option_with_args_text);
      break;

    case OPT_no_canonical_prefixes:
      /* Already handled as a special case, so ignored here.  */
      do_save = false;
      break;

    case OPT_pipe:
      validated = true;
      /* These options set the variables specified in common.opt
	 automatically, but do need to be saved for spec
	 processing.  */
      break;

    case OPT_specs_:
      {
	struct user_specs *user = XNEW (struct user_specs);

	user->next = (struct user_specs *) 0;
	user->filename = arg;
	if (user_specs_tail)
	  user_specs_tail->next = user;
	else
	  user_specs_head = user;
	user_specs_tail = user;
      }
      validated = true;
      break;

    case OPT__sysroot_:
      target_system_root = arg;
      target_system_root_changed = 1;
      do_save = false;
      break;

    case OPT_time_:
      if (report_times_to_file)
	fclose (report_times_to_file);
      report_times_to_file = fopen (arg, "a");
      do_save = false;
      break;

    case OPT____:
      /* "-###"
	 This is similar to -v except that there is no execution
	 of the commands and the echoed arguments are quoted.  It
	 is intended for use in shell scripts to capture the
	 driver-generated command line.  */
      verbose_only_flag++;
      verbose_flag = 1;
      do_save = false;
      break;

    case OPT_B:
      {
	size_t len = strlen (arg);

	/* Catch the case where the user has forgotten to append a
	   directory separator to the path.  Note, they may be using
	   -B to add an executable name prefix, eg "i386-elf-", in
	   order to distinguish between multiple installations of
	   GCC in the same directory.  Hence we must check to see
	   if appending a directory separator actually makes a
	   valid directory name.  */
	if (!IS_DIR_SEPARATOR (arg[len - 1])
	    && is_directory (arg, false))
	  {
	    char *tmp = XNEWVEC (char, len + 2);
	    strcpy (tmp, arg);
	    tmp[len] = DIR_SEPARATOR;
	    tmp[++len] = 0;
	    arg = tmp;
	  }

	add_prefix (&exec_prefixes, arg, NULL,
		    PREFIX_PRIORITY_B_OPT, 0, 0);
	add_prefix (&startfile_prefixes, arg, NULL,
		    PREFIX_PRIORITY_B_OPT, 0, 0);
	add_prefix (&include_prefixes, arg, NULL,
		    PREFIX_PRIORITY_B_OPT, 0, 0);
      }
      validated = true;
      break;

    case OPT_E:
      have_E = true;
      break;

    case OPT_x:
      spec_lang = arg;
      if (!strcmp (spec_lang, "none"))
	/* Suppress the warning if -xnone comes after the last input
	   file, because alternate command interfaces like g++ might
	   find it useful to place -xnone after each input file.  */
	spec_lang = 0;
      else
	last_language_n_infiles = n_infiles;
      do_save = false;
      break;

    case OPT_o:
      have_o = 1;
#if defined(HAVE_TARGET_EXECUTABLE_SUFFIX) || defined(HAVE_TARGET_OBJECT_SUFFIX)
      arg = convert_filename (arg, ! have_c, 0);
#endif
      output_file = arg;
      /* Save the output name in case -save-temps=obj was used.  */
      save_temps_prefix = xstrdup (arg);
      /* On some systems, ld cannot handle "-o" without a space.  So
	 split the option from its argument.  */
      save_switch ("-o", 1, &arg, validated, true);
      return true;

#ifdef ENABLE_DEFAULT_PIE
    case OPT_pie:
      /* -pie is turned on by default.  */
#endif

    case OPT_static_libgcc:
    case OPT_shared_libgcc:
    case OPT_static_libgfortran:
    case OPT_static_libstdc__:
      /* These are always valid, since gcc.c itself understands the
	 first two, gfortranspec.c understands -static-libgfortran and
	 g++spec.c understands -static-libstdc++ */
      validated = true;
      break;

    case OPT_fwpa:
      flag_wpa = "";
      break;

    case OPT_foffload_:
      handle_foffload_option (arg);
      break;

    default:
      /* Various driver options need no special processing at this
	 point, having been handled in a prescan above or being
	 handled by specs.  */
      break;
    }

  if (do_save)
    save_switch (decoded->canonical_option[0],
		 decoded->canonical_option_num_elements - 1,
		 &decoded->canonical_option[1], validated, true);
  return true;
}

/* Put the driver's standard set of option handlers in *HANDLERS.  */

static void
set_option_handlers (struct cl_option_handlers *handlers)
{
  handlers->unknown_option_callback = driver_unknown_option_callback;
  handlers->wrong_lang_callback = driver_wrong_lang_callback;
  handlers->num_handlers = 3;
  handlers->handlers[0].handler = driver_handle_option;
  handlers->handlers[0].mask = CL_DRIVER;
  handlers->handlers[1].handler = common_handle_option;
  handlers->handlers[1].mask = CL_COMMON;
  handlers->handlers[2].handler = target_handle_option;
  handlers->handlers[2].mask = CL_TARGET;
}

/* Create the vector `switches' and its contents.
   Store its length in `n_switches'.  */

static void
process_command (unsigned int decoded_options_count,
		 struct cl_decoded_option *decoded_options)
{
  const char *temp;
  char *temp1;
  char *tooldir_prefix, *tooldir_prefix2;
  char *(*get_relative_prefix) (const char *, const char *,
				const char *) = NULL;
  struct cl_option_handlers handlers;
  unsigned int j;

  gcc_exec_prefix = env.get ("GCC_EXEC_PREFIX");

  n_switches = 0;
  n_infiles = 0;
  added_libraries = 0;

  /* Figure compiler version from version string.  */

  compiler_version = temp1 = xstrdup (version_string);

  for (; *temp1; ++temp1)
    {
      if (*temp1 == ' ')
	{
	  *temp1 = '\0';
	  break;
	}
    }

  /* Handle any -no-canonical-prefixes flag early, to assign the function
     that builds relative prefixes.  This function creates default search
     paths that are needed later in normal option handling.  */

  for (j = 1; j < decoded_options_count; j++)
    {
      if (decoded_options[j].opt_index == OPT_no_canonical_prefixes)
	{
	  get_relative_prefix = make_relative_prefix_ignore_links;
	  break;
	}
    }
  if (! get_relative_prefix)
    get_relative_prefix = make_relative_prefix;

  /* Set up the default search paths.  If there is no GCC_EXEC_PREFIX,
     see if we can create it from the pathname specified in
     decoded_options[0].arg.  */

  gcc_libexec_prefix = standard_libexec_prefix;
#ifndef VMS
  /* FIXME: make_relative_prefix doesn't yet work for VMS.  */
  if (!gcc_exec_prefix)
    {
      gcc_exec_prefix = get_relative_prefix (decoded_options[0].arg,
					     standard_bindir_prefix,
					     standard_exec_prefix);
      gcc_libexec_prefix = get_relative_prefix (decoded_options[0].arg,
					     standard_bindir_prefix,
					     standard_libexec_prefix);
      if (gcc_exec_prefix)
	xputenv (concat ("GCC_EXEC_PREFIX=", gcc_exec_prefix, NULL));
    }
  else
    {
      /* make_relative_prefix requires a program name, but
	 GCC_EXEC_PREFIX is typically a directory name with a trailing
	 / (which is ignored by make_relative_prefix), so append a
	 program name.  */
      char *tmp_prefix = concat (gcc_exec_prefix, "gcc", NULL);
      gcc_libexec_prefix = get_relative_prefix (tmp_prefix,
						standard_exec_prefix,
						standard_libexec_prefix);

      /* The path is unrelocated, so fallback to the original setting.  */
      if (!gcc_libexec_prefix)
	gcc_libexec_prefix = standard_libexec_prefix;

      free (tmp_prefix);
    }
#else
#endif
  /* From this point onward, gcc_exec_prefix is non-null if the toolchain
     is relocated. The toolchain was either relocated using GCC_EXEC_PREFIX
     or an automatically created GCC_EXEC_PREFIX from
     decoded_options[0].arg.  */

  /* Do language-specific adjustment/addition of flags.  */
  lang_specific_driver (&decoded_options, &decoded_options_count,
			&added_libraries);

  if (gcc_exec_prefix)
    {
      int len = strlen (gcc_exec_prefix);

      if (len > (int) sizeof ("/lib/gcc/") - 1
	  && (IS_DIR_SEPARATOR (gcc_exec_prefix[len-1])))
	{
	  temp = gcc_exec_prefix + len - sizeof ("/lib/gcc/") + 1;
	  if (IS_DIR_SEPARATOR (*temp)
	      && filename_ncmp (temp + 1, "lib", 3) == 0
	      && IS_DIR_SEPARATOR (temp[4])
	      && filename_ncmp (temp + 5, "gcc", 3) == 0)
	    len -= sizeof ("/lib/gcc/") - 1;
	}

      set_std_prefix (gcc_exec_prefix, len);
      add_prefix (&exec_prefixes, gcc_libexec_prefix, "GCC",
		  PREFIX_PRIORITY_LAST, 0, 0);
      add_prefix (&startfile_prefixes, gcc_exec_prefix, "GCC",
		  PREFIX_PRIORITY_LAST, 0, 0);
    }

  /* COMPILER_PATH and LIBRARY_PATH have values
     that are lists of directory names with colons.  */

  temp = env.get ("COMPILER_PATH");
  if (temp)
    {
      const char *startp, *endp;
      char *nstore = (char *) alloca (strlen (temp) + 3);

      startp = endp = temp;
      while (1)
	{
	  if (*endp == PATH_SEPARATOR || *endp == 0)
	    {
	      strncpy (nstore, startp, endp - startp);
	      if (endp == startp)
		strcpy (nstore, concat (".", dir_separator_str, NULL));
	      else if (!IS_DIR_SEPARATOR (endp[-1]))
		{
		  nstore[endp - startp] = DIR_SEPARATOR;
		  nstore[endp - startp + 1] = 0;
		}
	      else
		nstore[endp - startp] = 0;
	      add_prefix (&exec_prefixes, nstore, 0,
			  PREFIX_PRIORITY_LAST, 0, 0);
	      add_prefix (&include_prefixes, nstore, 0,
			  PREFIX_PRIORITY_LAST, 0, 0);
	      if (*endp == 0)
		break;
	      endp = startp = endp + 1;
	    }
	  else
	    endp++;
	}
    }

  temp = env.get (LIBRARY_PATH_ENV);
  if (temp && *cross_compile == '0')
    {
      const char *startp, *endp;
      char *nstore = (char *) alloca (strlen (temp) + 3);

      startp = endp = temp;
      while (1)
	{
	  if (*endp == PATH_SEPARATOR || *endp == 0)
	    {
	      strncpy (nstore, startp, endp - startp);
	      if (endp == startp)
		strcpy (nstore, concat (".", dir_separator_str, NULL));
	      else if (!IS_DIR_SEPARATOR (endp[-1]))
		{
		  nstore[endp - startp] = DIR_SEPARATOR;
		  nstore[endp - startp + 1] = 0;
		}
	      else
		nstore[endp - startp] = 0;
	      add_prefix (&startfile_prefixes, nstore, NULL,
			  PREFIX_PRIORITY_LAST, 0, 1);
	      if (*endp == 0)
		break;
	      endp = startp = endp + 1;
	    }
	  else
	    endp++;
	}
    }

  /* Use LPATH like LIBRARY_PATH (for the CMU build program).  */
  temp = env.get ("LPATH");
  if (temp && *cross_compile == '0')
    {
      const char *startp, *endp;
      char *nstore = (char *) alloca (strlen (temp) + 3);

      startp = endp = temp;
      while (1)
	{
	  if (*endp == PATH_SEPARATOR || *endp == 0)
	    {
	      strncpy (nstore, startp, endp - startp);
	      if (endp == startp)
		strcpy (nstore, concat (".", dir_separator_str, NULL));
	      else if (!IS_DIR_SEPARATOR (endp[-1]))
		{
		  nstore[endp - startp] = DIR_SEPARATOR;
		  nstore[endp - startp + 1] = 0;
		}
	      else
		nstore[endp - startp] = 0;
	      add_prefix (&startfile_prefixes, nstore, NULL,
			  PREFIX_PRIORITY_LAST, 0, 1);
	      if (*endp == 0)
		break;
	      endp = startp = endp + 1;
	    }
	  else
	    endp++;
	}
    }

  /* Process the options and store input files and switches in their
     vectors.  */

  last_language_n_infiles = -1;

  set_option_handlers (&handlers);

  for (j = 1; j < decoded_options_count; j++)
    {
      switch (decoded_options[j].opt_index)
	{
	case OPT_S:
	case OPT_c:
	case OPT_E:
	  have_c = 1;
	  break;
	}
      if (have_c)
	break;
    }

  for (j = 1; j < decoded_options_count; j++)
    {
      if (decoded_options[j].opt_index == OPT_SPECIAL_input_file)
	{
	  const char *arg = decoded_options[j].arg;
          const char *p = strrchr (arg, '@');
          char *fname;
	  long offset;
	  int consumed;
#ifdef HAVE_TARGET_OBJECT_SUFFIX
	  arg = convert_filename (arg, 0, access (arg, F_OK));
#endif
	  /* For LTO static archive support we handle input file
	     specifications that are composed of a filename and
	     an offset like FNAME@OFFSET.  */
	  if (p
	      && p != arg
	      && sscanf (p, "@%li%n", &offset, &consumed) >= 1
	      && strlen (p) == (unsigned int)consumed)
	    {
              fname = (char *)xmalloc (p - arg + 1);
              memcpy (fname, arg, p - arg);
              fname[p - arg] = '\0';
	      /* Only accept non-stdin and existing FNAME parts, otherwise
		 try with the full name.  */
	      if (strcmp (fname, "-") == 0 || access (fname, F_OK) < 0)
		{
		  free (fname);
		  fname = xstrdup (arg);
		}
	    }
	  else
	    fname = xstrdup (arg);

          if (strcmp (fname, "-") != 0 && access (fname, F_OK) < 0)
	    {
	      bool resp = fname[0] == '@' && access (fname + 1, F_OK) < 0;
	      error ("%s: %m", fname + resp);
	    }
          else
	    add_infile (arg, spec_lang);

          free (fname);
	  continue;
	}

      read_cmdline_option (&global_options, &global_options_set,
			   decoded_options + j, UNKNOWN_LOCATION,
			   CL_DRIVER, &handlers, global_dc);
    }

  /* If the user didn't specify any, default to all configured offload
     targets.  */
  if (ENABLE_OFFLOADING && offload_targets == NULL)
    handle_foffload_option (OFFLOAD_TARGETS);

  if (output_file
      && strcmp (output_file, "-") != 0
      && strcmp (output_file, HOST_BIT_BUCKET) != 0)
    {
      int i;
      for (i = 0; i < n_infiles; i++)
	if ((!infiles[i].language || infiles[i].language[0] != '*')
	    && canonical_filename_eq (infiles[i].name, output_file))
	  fatal_error (input_location,
		       "input file %qs is the same as output file",
		       output_file);
    }

  if (output_file != NULL && output_file[0] == '\0')
    fatal_error (input_location, "output filename may not be empty");

  /* If -save-temps=obj and -o name, create the prefix to use for %b.
     Otherwise just make -save-temps=obj the same as -save-temps=cwd.  */
  if (save_temps_flag == SAVE_TEMPS_OBJ && save_temps_prefix != NULL)
    {
      save_temps_length = strlen (save_temps_prefix);
      temp = strrchr (lbasename (save_temps_prefix), '.');
      if (temp)
	{
	  save_temps_length -= strlen (temp);
	  save_temps_prefix[save_temps_length] = '\0';
	}

    }
  else if (save_temps_prefix != NULL)
    {
      free (save_temps_prefix);
      save_temps_prefix = NULL;
    }

  if (save_temps_flag && use_pipes)
    {
      /* -save-temps overrides -pipe, so that temp files are produced */
      if (save_temps_flag)
	warning (0, "%<-pipe%> ignored because %<-save-temps%> specified");
      use_pipes = 0;
    }

  if (!compare_debug)
    {
      const char *gcd = env.get ("GCC_COMPARE_DEBUG");

      if (gcd && gcd[0] == '-')
	{
	  compare_debug = 2;
	  compare_debug_opt = gcd;
	}
      else if (gcd && *gcd && strcmp (gcd, "0"))
	{
	  compare_debug = 3;
	  compare_debug_opt = "-gtoggle";
	}
    }
  else if (compare_debug < 0)
    {
      compare_debug = 0;
      gcc_assert (!compare_debug_opt);
    }

  /* Set up the search paths.  We add directories that we expect to
     contain GNU Toolchain components before directories specified by
     the machine description so that we will find GNU components (like
     the GNU assembler) before those of the host system.  */

  /* If we don't know where the toolchain has been installed, use the
     configured-in locations.  */
  if (!gcc_exec_prefix)
    {
#ifndef OS2
      add_prefix (&exec_prefixes, standard_libexec_prefix, "GCC",
		  PREFIX_PRIORITY_LAST, 1, 0);
      add_prefix (&exec_prefixes, standard_libexec_prefix, "BINUTILS",
		  PREFIX_PRIORITY_LAST, 2, 0);
      add_prefix (&exec_prefixes, standard_exec_prefix, "BINUTILS",
		  PREFIX_PRIORITY_LAST, 2, 0);
#endif
      add_prefix (&startfile_prefixes, standard_exec_prefix, "BINUTILS",
		  PREFIX_PRIORITY_LAST, 1, 0);
    }

  gcc_assert (!IS_ABSOLUTE_PATH (tooldir_base_prefix));
  tooldir_prefix2 = concat (tooldir_base_prefix, spec_machine,
			    dir_separator_str, NULL);

  /* Look for tools relative to the location from which the driver is
     running, or, if that is not available, the configured prefix.  */
  tooldir_prefix
    = concat (gcc_exec_prefix ? gcc_exec_prefix : standard_exec_prefix,
	      spec_host_machine, dir_separator_str, spec_version,
	      accel_dir_suffix, dir_separator_str, tooldir_prefix2, NULL);
  free (tooldir_prefix2);

  add_prefix (&exec_prefixes,
	      concat (tooldir_prefix, "bin", dir_separator_str, NULL),
	      "BINUTILS", PREFIX_PRIORITY_LAST, 0, 0);
  add_prefix (&startfile_prefixes,
	      concat (tooldir_prefix, "lib", dir_separator_str, NULL),
	      "BINUTILS", PREFIX_PRIORITY_LAST, 0, 1);
  free (tooldir_prefix);

#if defined(TARGET_SYSTEM_ROOT_RELOCATABLE) && !defined(VMS)
  /* If the normal TARGET_SYSTEM_ROOT is inside of $exec_prefix,
     then consider it to relocate with the rest of the GCC installation
     if GCC_EXEC_PREFIX is set.
     ``make_relative_prefix'' is not compiled for VMS, so don't call it.  */
  if (target_system_root && !target_system_root_changed && gcc_exec_prefix)
    {
      char *tmp_prefix = get_relative_prefix (decoded_options[0].arg,
					      standard_bindir_prefix,
					      target_system_root);
      if (tmp_prefix && access_check (tmp_prefix, F_OK) == 0)
	{
	  target_system_root = tmp_prefix;
	  target_system_root_changed = 1;
	}
    }
#endif

  /* More prefixes are enabled in main, after we read the specs file
     and determine whether this is cross-compilation or not.  */

  if (n_infiles != 0 && n_infiles == last_language_n_infiles && spec_lang != 0)
    warning (0, "%<-x %s%> after last input file has no effect", spec_lang);

  /* Synthesize -fcompare-debug flag from the GCC_COMPARE_DEBUG
     environment variable.  */
  if (compare_debug == 2 || compare_debug == 3)
    {
      const char *opt = concat ("-fcompare-debug=", compare_debug_opt, NULL);
      save_switch (opt, 0, NULL, false, true);
      compare_debug = 1;
    }

  /* Ensure we only invoke each subprocess once.  */
  if (n_infiles == 0
      && (print_subprocess_help || print_help_list || print_version))
    {
      /* Create a dummy input file, so that we can pass
	 the help option on to the various sub-processes.  */
      add_infile ("help-dummy", "c");
    }

  /* Decide if undefined variable references are allowed in specs.  */

  /* -v alone is safe. --version and --help alone or together are safe.  Note
     that -v would make them unsafe, as they'd then be run for subprocesses as
     well, the location of which might depend on variables possibly coming
     from self-specs.  Note also that the command name is counted in
     decoded_options_count.  */

  unsigned help_version_count = 0;

  if (print_version)
    help_version_count++;

  if (print_help_list)
    help_version_count++;

  spec_undefvar_allowed =
    ((verbose_flag && decoded_options_count == 2)
     || help_version_count == decoded_options_count - 1);

  alloc_switch ();
  switches[n_switches].part1 = 0;
  alloc_infile ();
  infiles[n_infiles].name = 0;
}

/* Store switches not filtered out by %<S in spec in COLLECT_GCC_OPTIONS
   and place that in the environment.  */

static void
set_collect_gcc_options (void)
{
  int i;
  int first_time;

  /* Build COLLECT_GCC_OPTIONS to have all of the options specified to
     the compiler.  */
  obstack_grow (&collect_obstack, "COLLECT_GCC_OPTIONS=",
		sizeof ("COLLECT_GCC_OPTIONS=") - 1);

  first_time = TRUE;
  for (i = 0; (int) i < n_switches; i++)
    {
      const char *const *args;
      const char *p, *q;
      if (!first_time)
	obstack_grow (&collect_obstack, " ", 1);

      first_time = FALSE;

      /* Ignore elided switches.  */
      if ((switches[i].live_cond
	   & (SWITCH_IGNORE | SWITCH_KEEP_FOR_GCC))
	  == SWITCH_IGNORE)
	continue;

      obstack_grow (&collect_obstack, "'-", 2);
      q = switches[i].part1;
      while ((p = strchr (q, '\'')))
	{
	  obstack_grow (&collect_obstack, q, p - q);
	  obstack_grow (&collect_obstack, "'\\''", 4);
	  q = ++p;
	}
      obstack_grow (&collect_obstack, q, strlen (q));
      obstack_grow (&collect_obstack, "'", 1);

      for (args = switches[i].args; args && *args; args++)
	{
	  obstack_grow (&collect_obstack, " '", 2);
	  q = *args;
	  while ((p = strchr (q, '\'')))
	    {
	      obstack_grow (&collect_obstack, q, p - q);
	      obstack_grow (&collect_obstack, "'\\''", 4);
	      q = ++p;
	    }
	  obstack_grow (&collect_obstack, q, strlen (q));
	  obstack_grow (&collect_obstack, "'", 1);
	}
    }
  obstack_grow (&collect_obstack, "\0", 1);
  xputenv (XOBFINISH (&collect_obstack, char *));
}

/* Process a spec string, accumulating and running commands.  */

/* These variables describe the input file name.
   input_file_number is the index on outfiles of this file,
   so that the output file name can be stored for later use by %o.
   input_basename is the start of the part of the input file
   sans all directory names, and basename_length is the number
   of characters starting there excluding the suffix .c or whatever.  */

static const char *gcc_input_filename;
static int input_file_number;
size_t input_filename_length;
static int basename_length;
static int suffixed_basename_length;
static const char *input_basename;
static const char *input_suffix;
#ifndef HOST_LACKS_INODE_NUMBERS
static struct stat input_stat;
#endif
static int input_stat_set;

/* The compiler used to process the current input file.  */
static struct compiler *input_file_compiler;

/* These are variables used within do_spec and do_spec_1.  */

/* Nonzero if an arg has been started and not yet terminated
   (with space, tab or newline).  */
static int arg_going;

/* Nonzero means %d or %g has been seen; the next arg to be terminated
   is a temporary file name.  */
static int delete_this_arg;

/* Nonzero means %w has been seen; the next arg to be terminated
   is the output file name of this compilation.  */
static int this_is_output_file;

/* Nonzero means %s has been seen; the next arg to be terminated
   is the name of a library file and we should try the standard
   search dirs for it.  */
static int this_is_library_file;

/* Nonzero means %T has been seen; the next arg to be terminated
   is the name of a linker script and we should try all of the
   standard search dirs for it.  If it is found insert a --script
   command line switch and then substitute the full path in place,
   otherwise generate an error message.  */
static int this_is_linker_script;

/* Nonzero means that the input of this command is coming from a pipe.  */
static int input_from_pipe;

/* Nonnull means substitute this for any suffix when outputting a switches
   arguments.  */
static const char *suffix_subst;

/* If there is an argument being accumulated, terminate it and store it.  */

static void
end_going_arg (void)
{
  if (arg_going)
    {
      const char *string;

      obstack_1grow (&obstack, 0);
      string = XOBFINISH (&obstack, const char *);
      if (this_is_library_file)
	string = find_file (string);
      if (this_is_linker_script)
	{
	  char * full_script_path = find_a_file (&startfile_prefixes, string, R_OK, true);

	  if (full_script_path == NULL)
	    {
	      error ("unable to locate default linker script %qs in the library search paths", string);
	      /* Script was not found on search path.  */
	      return;
	    }
	  store_arg ("--script", false, false);
	  string = full_script_path;
	}
      store_arg (string, delete_this_arg, this_is_output_file);
      if (this_is_output_file)
	outfiles[input_file_number] = string;
      arg_going = 0;
    }
}


/* Parse the WRAPPER string which is a comma separated list of the command line
   and insert them into the beginning of argbuf.  */

static void
insert_wrapper (const char *wrapper)
{
  int n = 0;
  int i;
  char *buf = xstrdup (wrapper);
  char *p = buf;
  unsigned int old_length = argbuf.length ();

  do
    {
      n++;
      while (*p == ',')
        p++;
    }
  while ((p = strchr (p, ',')) != NULL);

  argbuf.safe_grow (old_length + n);
  memmove (argbuf.address () + n,
	   argbuf.address (),
	   old_length * sizeof (const_char_p));

  i = 0;
  p = buf;
  do
    {
      while (*p == ',')
        {
          *p = 0;
          p++;
        }
      argbuf[i] = p;
      i++;
    }
  while ((p = strchr (p, ',')) != NULL);
  gcc_assert (i == n);
}

/* Process the spec SPEC and run the commands specified therein.
   Returns 0 if the spec is successfully processed; -1 if failed.  */

int
do_spec (const char *spec)
{
  int value;

  value = do_spec_2 (spec, NULL);

  /* Force out any unfinished command.
     If -pipe, this forces out the last command if it ended in `|'.  */
  if (value == 0)
    {
      if (argbuf.length () > 0
	  && !strcmp (argbuf.last (), "|"))
	argbuf.pop ();

      set_collect_gcc_options ();

      if (argbuf.length () > 0)
	value = execute ();
    }

  return value;
}

/* Process the spec SPEC, with SOFT_MATCHED_PART designating the current value
   of a matched * pattern which may be re-injected by way of %*.  */

static int
do_spec_2 (const char *spec, const char *soft_matched_part)
{
  int result;

  clear_args ();
  arg_going = 0;
  delete_this_arg = 0;
  this_is_output_file = 0;
  this_is_library_file = 0;
  this_is_linker_script = 0;
  input_from_pipe = 0;
  suffix_subst = NULL;

  result = do_spec_1 (spec, 0, soft_matched_part);

  end_going_arg ();

  return result;
}

/* Process the given spec string and add any new options to the end
   of the switches/n_switches array.  */

static void
do_option_spec (const char *name, const char *spec)
{
  unsigned int i, value_count, value_len;
  const char *p, *q, *value;
  char *tmp_spec, *tmp_spec_p;

  if (configure_default_options[0].name == NULL)
    return;

  for (i = 0; i < ARRAY_SIZE (configure_default_options); i++)
    if (strcmp (configure_default_options[i].name, name) == 0)
      break;
  if (i == ARRAY_SIZE (configure_default_options))
    return;

  value = configure_default_options[i].value;
  value_len = strlen (value);

  /* Compute the size of the final spec.  */
  value_count = 0;
  p = spec;
  while ((p = strstr (p, "%(VALUE)")) != NULL)
    {
      p ++;
      value_count ++;
    }

  /* Replace each %(VALUE) by the specified value.  */
  tmp_spec = (char *) alloca (strlen (spec) + 1
		     + value_count * (value_len - strlen ("%(VALUE)")));
  tmp_spec_p = tmp_spec;
  q = spec;
  while ((p = strstr (q, "%(VALUE)")) != NULL)
    {
      memcpy (tmp_spec_p, q, p - q);
      tmp_spec_p = tmp_spec_p + (p - q);
      memcpy (tmp_spec_p, value, value_len);
      tmp_spec_p += value_len;
      q = p + strlen ("%(VALUE)");
    }
  strcpy (tmp_spec_p, q);

  do_self_spec (tmp_spec);
}

/* Process the given spec string and add any new options to the end
   of the switches/n_switches array.  */

static void
do_self_spec (const char *spec)
{
  int i;

  do_spec_2 (spec, NULL);
  do_spec_1 (" ", 0, NULL);

  /* Mark %<S switches processed by do_self_spec to be ignored permanently.
     do_self_specs adds the replacements to switches array, so it shouldn't
     be processed afterwards.  */
  for (i = 0; i < n_switches; i++)
    if ((switches[i].live_cond & SWITCH_IGNORE))
      switches[i].live_cond |= SWITCH_IGNORE_PERMANENTLY;

  if (argbuf.length () > 0)
    {
      const char **argbuf_copy;
      struct cl_decoded_option *decoded_options;
      struct cl_option_handlers handlers;
      unsigned int decoded_options_count;
      unsigned int j;

      /* Create a copy of argbuf with a dummy argv[0] entry for
	 decode_cmdline_options_to_array.  */
      argbuf_copy = XNEWVEC (const char *,
			     argbuf.length () + 1);
      argbuf_copy[0] = "";
      memcpy (argbuf_copy + 1, argbuf.address (),
	      argbuf.length () * sizeof (const char *));

      decode_cmdline_options_to_array (argbuf.length () + 1,
				       argbuf_copy,
				       CL_DRIVER, &decoded_options,
				       &decoded_options_count);
      free (argbuf_copy);

      set_option_handlers (&handlers);

      for (j = 1; j < decoded_options_count; j++)
	{
	  switch (decoded_options[j].opt_index)
	    {
	    case OPT_SPECIAL_input_file:
	      /* Specs should only generate options, not input
		 files.  */
	      if (strcmp (decoded_options[j].arg, "-") != 0)
		fatal_error (input_location,
			     "switch %qs does not start with %<-%>",
			     decoded_options[j].arg);
	      else
		fatal_error (input_location,
			     "spec-generated switch is just %<-%>");
	      break;

	    case OPT_fcompare_debug_second:
	    case OPT_fcompare_debug:
	    case OPT_fcompare_debug_:
	    case OPT_o:
	      /* Avoid duplicate processing of some options from
		 compare-debug specs; just save them here.  */
	      save_switch (decoded_options[j].canonical_option[0],
			   (decoded_options[j].canonical_option_num_elements
			    - 1),
			   &decoded_options[j].canonical_option[1], false, true);
	      break;

	    default:
	      read_cmdline_option (&global_options, &global_options_set,
				   decoded_options + j, UNKNOWN_LOCATION,
				   CL_DRIVER, &handlers, global_dc);
	      break;
	    }
	}

      free (decoded_options);

      alloc_switch ();
      switches[n_switches].part1 = 0;
    }
}

/* Callback for processing %D and %I specs.  */

struct spec_path_info {
  const char *option;
  const char *append;
  size_t append_len;
  bool omit_relative;
  bool separate_options;
};

static void *
spec_path (char *path, void *data)
{
  struct spec_path_info *info = (struct spec_path_info *) data;
  size_t len = 0;
  char save = 0;

  if (info->omit_relative && !IS_ABSOLUTE_PATH (path))
    return NULL;

  if (info->append_len != 0)
    {
      len = strlen (path);
      memcpy (path + len, info->append, info->append_len + 1);
    }

  if (!is_directory (path, true))
    return NULL;

  do_spec_1 (info->option, 1, NULL);
  if (info->separate_options)
    do_spec_1 (" ", 0, NULL);

  if (info->append_len == 0)
    {
      len = strlen (path);
      save = path[len - 1];
      if (IS_DIR_SEPARATOR (path[len - 1]))
	path[len - 1] = '\0';
    }

  do_spec_1 (path, 1, NULL);
  do_spec_1 (" ", 0, NULL);

  /* Must not damage the original path.  */
  if (info->append_len == 0)
    path[len - 1] = save;

  return NULL;
}

/* True if we should compile INFILE. */

static bool
compile_input_file_p (struct infile *infile)
{
  if ((!infile->language) || (infile->language[0] != '*'))
    if (infile->incompiler == input_file_compiler)
      return true;
  return false;
}

/* Process each member of VEC as a spec.  */

static void
do_specs_vec (vec<char_p> vec)
{
  unsigned ix;
  char *opt;

  FOR_EACH_VEC_ELT (vec, ix, opt)
    {
      do_spec_1 (opt, 1, NULL);
      /* Make each accumulated option a separate argument.  */
      do_spec_1 (" ", 0, NULL);
    }
}

/* Add options passed via -Xassembler or -Wa to COLLECT_AS_OPTIONS.  */

static void
putenv_COLLECT_AS_OPTIONS (vec<char_p> vec)
{
  if (vec.is_empty ())
     return;

  obstack_init (&collect_obstack);
  obstack_grow (&collect_obstack, "COLLECT_AS_OPTIONS=",
		strlen ("COLLECT_AS_OPTIONS="));

  char *opt;
  unsigned ix;

  FOR_EACH_VEC_ELT (vec, ix, opt)
    {
      obstack_1grow (&collect_obstack, '\'');
      obstack_grow (&collect_obstack, opt, strlen (opt));
      obstack_1grow (&collect_obstack, '\'');
      if (ix < vec.length () - 1)
	obstack_1grow(&collect_obstack, ' ');
    }

  obstack_1grow (&collect_obstack, '\0');
  xputenv (XOBFINISH (&collect_obstack, char *));
}

/* Process the sub-spec SPEC as a portion of a larger spec.
   This is like processing a whole spec except that we do
   not initialize at the beginning and we do not supply a
   newline by default at the end.
   INSWITCH nonzero means don't process %-sequences in SPEC;
   in this case, % is treated as an ordinary character.
   This is used while substituting switches.
   INSWITCH nonzero also causes SPC not to terminate an argument.

   Value is zero unless a line was finished
   and the command on that line reported an error.  */

static int
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

		/* See if we already have an association of %g/%u/%U and
		   suffix.  */
		for (t = temp_names; t; t = t->next)
		  if (t->length == suffix_length
		      && strncmp (t->suffix, suffix, suffix_length) == 0
		      && t->unique == (c == 'u' || c == 'U' || c == 'j'))
		    break;

		/* Make a new association if needed.  %u and %j
		   require one.  */
		if (t == 0 || c == 'u' || c == 'j')
		  {
		    if (t == 0)
		      {
			t = XNEW (struct temp_name);
			t->next = temp_names;
			temp_names = t;
		      }
		    t->length = suffix_length;
		    if (saved_suffix)
		      {
			t->suffix = saved_suffix;
			saved_suffix = NULL;
		      }
		    else
		      t->suffix = save_string (suffix, suffix_length);
		    t->unique = (c == 'u' || c == 'U' || c == 'j');
		    temp_filename = make_temp_file (t->suffix);
		    temp_filename_length = strlen (temp_filename);
		    t->filename = temp_filename;
		    t->filename_length = temp_filename_length;
		  }

		free (saved_suffix);

		obstack_grow (&obstack, t->filename, t->filename_length);
		delete_this_arg = 1;
	      }
	    arg_going = 1;
	    break;

	  case 'i':
	    if (combine_inputs)
	      {
		/* We are going to expand `%i' into `@FILE', where FILE
		   is a newly-created temporary filename.  The filenames
		   that would usually be expanded in place of %o will be
		   written to the temporary file.  */
		if (at_file_supplied)
		  open_at_file ();

		for (i = 0; (int) i < n_infiles; i++)
		  if (compile_input_file_p (&infiles[i]))
		    {
		      store_arg (infiles[i].name, 0, 0);
		      infiles[i].compiled = true;
		    }

		if (at_file_supplied)
		  close_at_file ();
	      }
	    else
	      {
		obstack_grow (&obstack, gcc_input_filename,
			      input_filename_length);
		arg_going = 1;
	      }
	    break;

	  case 'I':
	    {
	      struct spec_path_info info;

	      if (multilib_dir)
		{
		  do_spec_1 ("-imultilib", 1, NULL);
		  /* Make this a separate argument.  */
		  do_spec_1 (" ", 0, NULL);
		  do_spec_1 (multilib_dir, 1, NULL);
		  do_spec_1 (" ", 0, NULL);
		}

	      if (multiarch_dir)
		{
		  do_spec_1 ("-imultiarch", 1, NULL);
		  /* Make this a separate argument.  */
		  do_spec_1 (" ", 0, NULL);
		  do_spec_1 (multiarch_dir, 1, NULL);
		  do_spec_1 (" ", 0, NULL);
		}

	      if (gcc_exec_prefix)
		{
		  do_spec_1 ("-iprefix", 1, NULL);
		  /* Make this a separate argument.  */
		  do_spec_1 (" ", 0, NULL);
		  do_spec_1 (gcc_exec_prefix, 1, NULL);
		  do_spec_1 (" ", 0, NULL);
		}

	      if (target_system_root_changed ||
		  (target_system_root && target_sysroot_hdrs_suffix))
		{
		  do_spec_1 ("-isysroot", 1, NULL);
		  /* Make this a separate argument.  */
		  do_spec_1 (" ", 0, NULL);
		  do_spec_1 (target_system_root, 1, NULL);
		  if (target_sysroot_hdrs_suffix)
		    do_spec_1 (target_sysroot_hdrs_suffix, 1, NULL);
		  do_spec_1 (" ", 0, NULL);
		}

	      info.option = "-isystem";
	      info.append = "include";
	      info.append_len = strlen (info.append);
	      info.omit_relative = false;
	      info.separate_options = true;

	      for_each_path (&include_prefixes, false, info.append_len,
			     spec_path, &info);

	      info.append = "include-fixed";
	      if (*sysroot_hdrs_suffix_spec)
		info.append = concat (info.append, dir_separator_str,
				      multilib_dir, NULL);
	      info.append_len = strlen (info.append);
	      for_each_path (&include_prefixes, false, info.append_len,
			     spec_path, &info);
	    }
	    break;

	  case 'o':
	    /* We are going to expand `%o' into `@FILE', where FILE
	       is a newly-created temporary filename.  The filenames
	       that would usually be expanded in place of %o will be
	       written to the temporary file.  */
	    if (at_file_supplied)
	      open_at_file ();

	    for (i = 0; i < n_infiles + lang_specific_extra_outfiles; i++)
	      if (outfiles[i])
		store_arg (outfiles[i], 0, 0);

	    if (at_file_supplied)
	      close_at_file ();
	    break;

	  case 'O':
	    obstack_grow (&obstack, TARGET_OBJECT_SUFFIX, strlen (TARGET_OBJECT_SUFFIX));
	    arg_going = 1;
	    break;

	  case 's':
	    this_is_library_file = 1;
	    break;

	  case 'T':
	    this_is_linker_script = 1;
	    break;

	  case 'V':
	    outfiles[input_file_number] = NULL;
	    break;

	  case 'w':
	    this_is_output_file = 1;
	    break;

	  case 'W':
	    {
	      unsigned int cur_index = argbuf.length ();
	      /* Handle the {...} following the %W.  */
	      if (*p != '{')
		fatal_error (input_location,
			     "spec %qs has invalid %<%%W%c%>", spec, *p);
	      p = handle_braces (p + 1);
	      if (p == 0)
		return -1;
	      end_going_arg ();
	      /* If any args were output, mark the last one for deletion
		 on failure.  */
	      if (argbuf.length () != cur_index)
		record_temp_file (argbuf.last (), 0, 1);
	      break;
	    }

	  case '@':
	    /* Handle the {...} following the %@.  */
	    if (*p != '{')
	      fatal_error (input_location,
			   "spec %qs has invalid %<%%@%c%>", spec, *p);
	    if (at_file_supplied)
	      open_at_file ();
	    p = handle_braces (p + 1);
	    if (at_file_supplied)
	      close_at_file ();
	    if (p == 0)
	      return -1;
	    break;

	  /* %x{OPTION} records OPTION for %X to output.  */
	  case 'x':
	    {
	      const char *p1 = p;
	      char *string;
	      char *opt;
	      unsigned ix;

	      /* Skip past the option value and make a copy.  */
	      if (*p != '{')
		fatal_error (input_location,
			     "spec %qs has invalid %<%%x%c%>", spec, *p);
	      while (*p++ != '}')
		;
	      string = save_string (p1 + 1, p - p1 - 2);

	      /* See if we already recorded this option.  */
	      FOR_EACH_VEC_ELT (linker_options, ix, opt)
		if (! strcmp (string, opt))
		  {
		    free (string);
		    return 0;
		  }

	      /* This option is new; add it.  */
	      add_linker_option (string, strlen (string));
	      free (string);
	    }
	    break;

	  /* Dump out the options accumulated previously using %x.  */
	  case 'X':
	    do_specs_vec (linker_options);
	    break;

	  /* Dump out the options accumulated previously using -Wa,.  */
	  case 'Y':
	    do_specs_vec (assembler_options);
	    break;

	  /* Dump out the options accumulated previously using -Wp,.  */
	  case 'Z':
	    do_specs_vec (preprocessor_options);
	    break;

	    /* Here are digits and numbers that just process
	       a certain constant string as a spec.  */

	  case '1':
	    value = do_spec_1 (cc1_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case '2':
	    value = do_spec_1 (cc1plus_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case 'a':
	    value = do_spec_1 (asm_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case 'A':
	    value = do_spec_1 (asm_final_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case 'C':
	    {
	      const char *const spec
		= (input_file_compiler->cpp_spec
		   ? input_file_compiler->cpp_spec
		   : cpp_spec);
	      value = do_spec_1 (spec, 0, NULL);
	      if (value != 0)
		return value;
	    }
	    break;

	  case 'E':
	    value = do_spec_1 (endfile_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case 'l':
	    value = do_spec_1 (link_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case 'L':
	    value = do_spec_1 (lib_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case 'M':
	    if (multilib_os_dir == NULL)
	      obstack_1grow (&obstack, '.');
	    else
	      obstack_grow (&obstack, multilib_os_dir,
			    strlen (multilib_os_dir));
	    break;

	  case 'G':
	    value = do_spec_1 (libgcc_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	  case 'R':
	    /* We assume there is a directory
	       separator at the end of this string.  */
	    if (target_system_root)
	      {
	        obstack_grow (&obstack, target_system_root,
			      strlen (target_system_root));
		if (target_sysroot_suffix)
		  obstack_grow (&obstack, target_sysroot_suffix,
				strlen (target_sysroot_suffix));
	      }
	    break;

	  case 'S':
	    value = do_spec_1 (startfile_spec, 0, NULL);
	    if (value != 0)
	      return value;
	    break;

	    /* Here we define characters other than letters and digits.  */

	  case '{':
	    p = handle_braces (p);
	    if (p == 0)
	      return -1;
	    break;

	  case ':':
	    p = handle_spec_function (p, NULL, soft_matched_part);
	    if (p == 0)
	      return -1;
	    break;

	  case '%':
	    obstack_1grow (&obstack, '%');
	    break;

	  case '.':
	    {
	      unsigned len = 0;

	      while (p[len] && p[len] != ' ' && p[len] != '%')
		len++;
	      suffix_subst = save_string (p - 1, len + 1);
	      p += len;
	    }
	   break;

	   /* Henceforth ignore the option(s) matching the pattern
	      after the %<.  */
	  case '<':
	  case '>':
	    {
	      unsigned len = 0;
	      int have_wildcard = 0;
	      int i;
	      int switch_option;

	      if (c == '>')
		switch_option = SWITCH_IGNORE | SWITCH_KEEP_FOR_GCC;
	      else
		switch_option = SWITCH_IGNORE;

	      while (p[len] && p[len] != ' ' && p[len] != '\t')
		len++;

	      if (p[len-1] == '*')
		have_wildcard = 1;

	      for (i = 0; i < n_switches; i++)
		if (!strncmp (switches[i].part1, p, len - have_wildcard)
		    && (have_wildcard || switches[i].part1[len] == '\0'))
		  {
		    switches[i].live_cond |= switch_option;
		    /* User switch be validated from validate_all_switches.
		       when the definition is seen from the spec file.
		       If not defined anywhere, will be rejected.  */
		    if (switches[i].known)
		      switches[i].validated = true;
		  }

	      p += len;
	    }
	    break;

	  case '*':
	    if (soft_matched_part)
	      {
		if (soft_matched_part[0])
		  do_spec_1 (soft_matched_part, 1, NULL);
		/* Only insert a space after the substitution if it is at the
		   end of the current sequence.  So if:

		     "%{foo=*:bar%*}%{foo=*:one%*two}"

		   matches -foo=hello then it will produce:
		   
		     barhello onehellotwo
		*/
		if (*p == 0 || *p == '}')
		  do_spec_1 (" ", 0, NULL);
	      }
	    else
	      /* Catch the case where a spec string contains something like
		 '%{foo:%*}'.  i.e. there is no * in the pattern on the left
		 hand side of the :.  */
	      error ("spec failure: %<%%*%> has not been initialized by pattern match");
	    break;

	    /* Process a string found as the value of a spec given by name.
	       This feature allows individual machine descriptions
	       to add and use their own specs.  */
	  case '(':
	    {
	      const char *name = p;
	      struct spec_list *sl;
	      int len;

	      /* The string after the S/P is the name of a spec that is to be
		 processed.  */
	      while (*p && *p != ')')
		p++;

	      /* See if it's in the list.  */
	      for (len = p - name, sl = specs; sl; sl = sl->next)
		if (sl->name_len == len && !strncmp (sl->name, name, len))
		  {
		    name = *(sl->ptr_spec);
#ifdef DEBUG_SPECS
		    fnotice (stderr, "Processing spec (%s), which is '%s'\n",
			     sl->name, name);
#endif
		    break;
		  }

	      if (sl)
		{
		  value = do_spec_1 (name, 0, NULL);
		  if (value != 0)
		    return value;
		}

	      /* Discard the closing paren.  */
	      if (*p)
		p++;
	    }
	    break;

	  default:
	    error ("spec failure: unrecognized spec option %qc", c);
	    break;
	  }
	break;

      case '\\':
	/* Backslash: treat next character as ordinary.  */
	c = *p++;

	/* Fall through.  */
      default:
	/* Ordinary character: put it into the current argument.  */
	obstack_1grow (&obstack, c);
	arg_going = 1;
      }

  /* End of string.  If we are processing a spec function, we need to
     end any pending argument.  */
  if (processing_spec_function)
    end_going_arg ();

  return 0;
}

/* Look up a spec function.  */

static const struct spec_function *
lookup_spec_function (const char *name)
{
  const struct spec_function *sf;

  for (sf = static_spec_functions; sf->name != NULL; sf++)
    if (strcmp (sf->name, name) == 0)
      return sf;

  return NULL;
}

/* Evaluate a spec function.  */

static const char *
eval_spec_function (const char *func, const char *args,
		    const char *soft_matched_part)
{
  const struct spec_function *sf;
  const char *funcval;

  /* Saved spec processing context.  */
  vec<const_char_p> save_argbuf;

  int save_arg_going;
  int save_delete_this_arg;
  int save_this_is_output_file;
  int save_this_is_library_file;
  int save_input_from_pipe;
  int save_this_is_linker_script;
  const char *save_suffix_subst;

  int save_growing_size;
  void *save_growing_value = NULL;

  sf = lookup_spec_function (func);
  if (sf == NULL)
    fatal_error (input_location, "unknown spec function %qs", func);

  /* Push the spec processing context.  */
  save_argbuf = argbuf;

  save_arg_going = arg_going;
  save_delete_this_arg = delete_this_arg;
  save_this_is_output_file = this_is_output_file;
  save_this_is_library_file = this_is_library_file;
  save_this_is_linker_script = this_is_linker_script;
  save_input_from_pipe = input_from_pipe;
  save_suffix_subst = suffix_subst;

  /* If we have some object growing now, finalize it so the args and function
     eval proceed from a cleared context.  This is needed to prevent the first
     constructed arg from mistakenly including the growing value.  We'll push
     this value back on the obstack once the function evaluation is done, to
     restore a consistent processing context for our caller.  This is fine as
     the address of growing objects isn't guaranteed to remain stable until
     they are finalized, and we expect this situation to be rare enough for
     the extra copy not to be an issue.  */
  save_growing_size = obstack_object_size (&obstack);
  if (save_growing_size > 0)
    save_growing_value = obstack_finish (&obstack);

  /* Create a new spec processing context, and build the function
     arguments.  */

  alloc_args ();
  if (do_spec_2 (args, soft_matched_part) < 0)
    fatal_error (input_location, "error in arguments to spec function %qs",
		 func);

  /* argbuf_index is an index for the next argument to be inserted, and
     so contains the count of the args already inserted.  */

  funcval = (*sf->func) (argbuf.length (),
			 argbuf.address ());

  /* Pop the spec processing context.  */
  argbuf.release ();
  argbuf = save_argbuf;

  arg_going = save_arg_going;
  delete_this_arg = save_delete_this_arg;
  this_is_output_file = save_this_is_output_file;
  this_is_library_file = save_this_is_library_file;
  this_is_linker_script = save_this_is_linker_script;
  input_from_pipe = save_input_from_pipe;
  suffix_subst = save_suffix_subst;

  if (save_growing_size > 0)
    obstack_grow (&obstack, save_growing_value, save_growing_size);

  return funcval;
}

/* Handle a spec function call of the form:

   %:function(args)

   ARGS is processed as a spec in a separate context and split into an
   argument vector in the normal fashion.  The function returns a string
   containing a spec which we then process in the caller's context, or
   NULL if no processing is required.

   If RETVAL_NONNULL is not NULL, then store a bool whether function
   returned non-NULL.

   SOFT_MATCHED_PART holds the current value of a matched * pattern, which
   may be re-expanded with a %* as part of the function arguments.  */

static const char *
handle_spec_function (const char *p, bool *retval_nonnull,
		      const char *soft_matched_part)
{
  char *func, *args;
  const char *endp, *funcval;
  int count;

  processing_spec_function++;

  /* Get the function name.  */
  for (endp = p; *endp != '\0'; endp++)
    {
      if (*endp == '(')		/* ) */
        break;
      /* Only allow [A-Za-z0-9], -, and _ in function names.  */
      if (!ISALNUM (*endp) && !(*endp == '-' || *endp == '_'))
	fatal_error (input_location, "malformed spec function name");
    }
  if (*endp != '(')		/* ) */
    fatal_error (input_location, "no arguments for spec function");
  func = save_string (p, endp - p);
  p = ++endp;

  /* Get the arguments.  */
  for (count = 0; *endp != '\0'; endp++)
    {
      /* ( */
      if (*endp == ')')
	{
	  if (count == 0)
	    break;
	  count--;
	}
      else if (*endp == '(')	/* ) */
	count++;
    }
  /* ( */
  if (*endp != ')')
    fatal_error (input_location, "malformed spec function arguments");
  args = save_string (p, endp - p);
  p = ++endp;

  /* p now points to just past the end of the spec function expression.  */

  funcval = eval_spec_function (func, args, soft_matched_part);
  if (funcval != NULL && do_spec_1 (funcval, 0, NULL) < 0)
    p = NULL;
  if (retval_nonnull)
    *retval_nonnull = funcval != NULL;

  free (func);
  free (args);

  processing_spec_function--;

  return p;
}

/* Inline subroutine of handle_braces.  Returns true if the current
   input suffix matches the atom bracketed by ATOM and END_ATOM.  */
static inline bool
input_suffix_matches (const char *atom, const char *end_atom)
{
  return (input_suffix
	  && !strncmp (input_suffix, atom, end_atom - atom)
	  && input_suffix[end_atom - atom] == '\0');
}

/* Subroutine of handle_braces.  Returns true if the current
   input file's spec name matches the atom bracketed by ATOM and END_ATOM.  */
static bool
input_spec_matches (const char *atom, const char *end_atom)
{
  return (input_file_compiler
	  && input_file_compiler->suffix
	  && input_file_compiler->suffix[0] != '\0'
	  && !strncmp (input_file_compiler->suffix + 1, atom,
		       end_atom - atom)
	  && input_file_compiler->suffix[end_atom - atom + 1] == '\0');
}

/* Subroutine of handle_braces.  Returns true if a switch
   matching the atom bracketed by ATOM and END_ATOM appeared on the
   command line.  */
static bool
switch_matches (const char *atom, const char *end_atom, int starred)
{
  int i;
  int len = end_atom - atom;
  int plen = starred ? len : -1;

  for (i = 0; i < n_switches; i++)
    if (!strncmp (switches[i].part1, atom, len)
	&& (starred || switches[i].part1[len] == '\0')
	&& check_live_switch (i, plen))
      return true;

    /* Check if a switch with separated form matching the atom.
       We check -D and -U switches. */
    else if (switches[i].args != 0)
      {
	if ((*switches[i].part1 == 'D' || *switches[i].part1 == 'U')
	    && *switches[i].part1 == atom[0])
	  {
	    if (!strncmp (switches[i].args[0], &atom[1], len - 1)
		&& (starred || (switches[i].part1[1] == '\0'
				&& switches[i].args[0][len - 1] == '\0'))
		&& check_live_switch (i, (starred ? 1 : -1)))
	      return true;
	  }
      }

  return false;
}

/* Inline subroutine of handle_braces.  Mark all of the switches which
   match ATOM (extends to END_ATOM; STARRED indicates whether there
   was a star after the atom) for later processing.  */
static inline void
mark_matching_switches (const char *atom, const char *end_atom, int starred)
{
  int i;
  int len = end_atom - atom;
  int plen = starred ? len : -1;

  for (i = 0; i < n_switches; i++)
    if (!strncmp (switches[i].part1, atom, len)
	&& (starred || switches[i].part1[len] == '\0')
	&& check_live_switch (i, plen))
      switches[i].ordering = 1;
}

/* Inline subroutine of handle_braces.  Process all the currently
   marked switches through give_switch, and clear the marks.  */
static inline void
process_marked_switches (void)
{
  int i;

  for (i = 0; i < n_switches; i++)
    if (switches[i].ordering == 1)
      {
	switches[i].ordering = 0;
	give_switch (i, 0);
      }
}

/* Handle a %{ ... } construct.  P points just inside the leading {.
   Returns a pointer one past the end of the brace block, or 0
   if we call do_spec_1 and that returns -1.  */

static const char *
handle_braces (const char *p)
{
  const char *atom, *end_atom;
  const char *d_atom = NULL, *d_end_atom = NULL;
  char *esc_buf = NULL, *d_esc_buf = NULL;
  int esc;
  const char *orig = p;

  bool a_is_suffix;
  bool a_is_spectype;
  bool a_is_starred;
  bool a_is_negated;
  bool a_matched;

  bool a_must_be_last = false;
  bool ordered_set    = false;
  bool disjunct_set   = false;
  bool disj_matched   = false;
  bool disj_starred   = true;
  bool n_way_choice   = false;
  bool n_way_matched  = false;

#define SKIP_WHITE() do { while (*p == ' ' || *p == '\t') p++; } while (0)

  do
    {
      if (a_must_be_last)
	goto invalid;

      /* Scan one "atom" (S in the description above of %{}, possibly
	 with '!', '.', '@', ',', or '*' modifiers).  */
      a_matched = false;
      a_is_suffix = false;
      a_is_starred = false;
      a_is_negated = false;
      a_is_spectype = false;

      SKIP_WHITE ();
      if (*p == '!')
	p++, a_is_negated = true;

      SKIP_WHITE ();
      if (*p == '%' && p[1] == ':')
	{
	  atom = NULL;
	  end_atom = NULL;
	  p = handle_spec_function (p + 2, &a_matched, NULL);
	}
      else
	{
	  if (*p == '.')
	    p++, a_is_suffix = true;
	  else if (*p == ',')
	    p++, a_is_spectype = true;

	  atom = p;
	  esc = 0;
	  while (ISIDNUM (*p) || *p == '-' || *p == '+' || *p == '='
		 || *p == ',' || *p == '.' || *p == '@' || *p == '\\')
	    {
	      if (*p == '\\')
		{
		  p++;
		  if (!*p)
		    fatal_error (input_location,
				 "braced spec %qs ends in escape", orig);
		  esc++;
		}
	      p++;
	    }
	  end_atom = p;

	  if (esc)
	    {
	      const char *ap;
	      char *ep;

	      if (esc_buf && esc_buf != d_esc_buf)
		free (esc_buf);
	      esc_buf = NULL;
	      ep = esc_buf = (char *) xmalloc (end_atom - atom - esc + 1);
	      for (ap = atom; ap != end_atom; ap++, ep++)
		{
		  if (*ap == '\\')
		    ap++;
		  *ep = *ap;
		}
	      *ep = '\0';
	      atom = esc_buf;
	      end_atom = ep;
	    }

	  if (*p == '*')
	    p++, a_is_starred = 1;
	}

      SKIP_WHITE ();
      switch (*p)
	{
	case '&': case '}':
	  /* Substitute the switch(es) indicated by the current atom.  */
	  ordered_set = true;
	  if (disjunct_set || n_way_choice || a_is_negated || a_is_suffix
	      || a_is_spectype || atom == end_atom)
	    goto invalid;

	  mark_matching_switches (atom, end_atom, a_is_starred);

	  if (*p == '}')
	    process_marked_switches ();
	  break;

	case '|': case ':':
	  /* Substitute some text if the current atom appears as a switch
	     or suffix.  */
	  disjunct_set = true;
	  if (ordered_set)
	    goto invalid;

	  if (atom && atom == end_atom)
	    {
	      if (!n_way_choice || disj_matched || *p == '|'
		  || a_is_negated || a_is_suffix || a_is_spectype
		  || a_is_starred)
		goto invalid;

	      /* An empty term may appear as the last choice of an
		 N-way choice set; it means "otherwise".  */
	      a_must_be_last = true;
	      disj_matched = !n_way_matched;
	      disj_starred = false;
	    }
	  else
	    {
	      if ((a_is_suffix || a_is_spectype) && a_is_starred)
		goto invalid;

	      if (!a_is_starred)
		disj_starred = false;

	      /* Don't bother testing this atom if we already have a
		 match.  */
	      if (!disj_matched && !n_way_matched)
		{
		  if (atom == NULL)
		    /* a_matched is already set by handle_spec_function.  */;
		  else if (a_is_suffix)
		    a_matched = input_suffix_matches (atom, end_atom);
		  else if (a_is_spectype)
		    a_matched = input_spec_matches (atom, end_atom);
		  else
		    a_matched = switch_matches (atom, end_atom, a_is_starred);

		  if (a_matched != a_is_negated)
		    {
		      disj_matched = true;
		      d_atom = atom;
		      d_end_atom = end_atom;
		      d_esc_buf = esc_buf;
		    }
		}
	    }

	  if (*p == ':')
	    {
	      /* Found the body, that is, the text to substitute if the
		 current disjunction matches.  */
	      p = process_brace_body (p + 1, d_atom, d_end_atom, disj_starred,
				      disj_matched && !n_way_matched);
	      if (p == 0)
		goto done;

	      /* If we have an N-way choice, reset state for the next
		 disjunction.  */
	      if (*p == ';')
		{
		  n_way_choice = true;
		  n_way_matched |= disj_matched;
		  disj_matched = false;
		  disj_starred = true;
		  d_atom = d_end_atom = NULL;
		}
	    }
	  break;

	default:
	  goto invalid;
	}
    }
  while (*p++ != '}');

 done:
  if (d_esc_buf && d_esc_buf != esc_buf)
    free (d_esc_buf);
  if (esc_buf)
    free (esc_buf);

  return p;

 invalid:
  fatal_error (input_location, "braced spec %qs is invalid at %qc", orig, *p);

#undef SKIP_WHITE
}

/* Subroutine of handle_braces.  Scan and process a brace substitution body
   (X in the description of %{} syntax).  P points one past the colon;
   ATOM and END_ATOM bracket the first atom which was found to be true
   (present) in the current disjunction; STARRED indicates whether all
   the atoms in the current disjunction were starred (for syntax validation);
   MATCHED indicates whether the disjunction matched or not, and therefore
   whether or not the body is to be processed through do_spec_1 or just
   skipped.  Returns a pointer to the closing } or ;, or 0 if do_spec_1


// Source: gcc.c
// Lines 1874-6579
