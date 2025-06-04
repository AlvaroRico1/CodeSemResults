void prepend_xassembler_to_collect_as_options (const char *collect_as_options,
					       obstack *o)
{
  obstack opts_obstack;
  int opts_count;

  obstack_init (&opts_obstack);
  parse_options_from_collect_gcc_options (collect_as_options,
					  &opts_obstack, &opts_count);
  const char **assembler_opts = XOBFINISH (&opts_obstack, const char **);

  for (int i = 0; i < opts_count; i++)
    {
      obstack_grow (o, " '-Xassembler' ",
		    strlen (" '-Xassembler' "));
      const char *opt = assembler_opts[i];
      obstack_1grow (o, '\'');
      obstack_grow (o, opt, strlen (opt));
      obstack_1grow (o, '\'');
    }


// Source: opts-common.c
// Lines 1787-1806
