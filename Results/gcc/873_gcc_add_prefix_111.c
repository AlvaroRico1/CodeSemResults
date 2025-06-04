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


// Source: gcc.c
// Lines 2912-2940
