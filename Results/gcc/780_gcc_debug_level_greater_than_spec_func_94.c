debug_level_greater_than_spec_func (int argc, const char **argv)
{
  char *converted;

  if (argc != 1)
    fatal_error (input_location,
		 "wrong number of arguments to %%:debug-level-gt");

  long arg = strtol (argv[0], &converted, 10);
  gcc_assert (converted != argv[0]);

  if (debug_info_level > arg)
    return "";

  return NULL;
}


// Source: gcc.c
// Lines 9999-10014
