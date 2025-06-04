print_version_one_plugin (void **slot, void *data)
{
  struct print_options *opt = (struct print_options *) data;
  struct plugin_name_args *plugin = (struct plugin_name_args *) *slot;
  const char *version = plugin->version ? plugin->version : "Unknown version.";

  fprintf (opt->file, " %s%s: %s\n", opt->indent, plugin->base_name, version);
  return 1;
}


// Source: plugin.c
// Lines 827-835
