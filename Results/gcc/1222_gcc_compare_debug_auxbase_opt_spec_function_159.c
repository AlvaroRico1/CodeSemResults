compare_debug_auxbase_opt_spec_function (int arg,
					 const char **argv)
{
  char *name;
  int len;

  if (arg == 0)
    fatal_error (input_location,
		 "too few arguments to %%:compare-debug-auxbase-opt");

  if (arg != 1)
    fatal_error (input_location,
		 "too many arguments to %%:compare-debug-auxbase-opt");

  if (compare_debug >= 0)
    return NULL;

  len = strlen (argv[0]);
  if (len < 3 || strcmp (argv[0] + len - 3, ".gk") != 0)
    fatal_error (input_location, "argument to %%:compare-debug-auxbase-opt "
		 "does not end in %<.gk%>");

  if (debug_auxbase_opt)
    return debug_auxbase_opt;

#define OPT "-auxbase "

  len -= 3;
  name = (char*) xmalloc (sizeof (OPT) + len);
  memcpy (name, OPT, sizeof (OPT) - 1);
  memcpy (name + sizeof (OPT) - 1, argv[0], len);
  name[sizeof (OPT) - 1 + len] = '\0';

#undef OPT

  return name;
}


// Source: gcc.c
// Lines 9859-9895
