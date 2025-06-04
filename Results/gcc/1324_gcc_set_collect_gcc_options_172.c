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


// Source: gcc.c
// Lines 4795-4845
