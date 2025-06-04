print_help_one_plugin (void **slot, void *data)
{
  struct print_options *opt = (struct print_options *) data;
  struct plugin_name_args *plugin = (struct plugin_name_args *) *slot;
  const char *help = plugin->help ? plugin->help : "No help available .";

  char *dup = xstrdup (help);
  char *p, *nl;
  fprintf (opt->file, " %s%s:\n", opt->indent, plugin->base_name);

  for (p = nl = dup; nl; p = nl)
    {
      nl = strchr (nl, '\n');
      if (nl)
	{
	  *nl = '\0';
	  nl++;
	}
      fprintf (opt->file, "   %s %s\n", opt->indent, p);
    }

  free (dup);
  return 1;
}


// Source: plugin.c
// Lines 856-879
