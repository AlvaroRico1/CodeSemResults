print_plugins_versions (FILE *file, const char *indent)
{
  struct print_options opt;
  opt.file = file;
  opt.indent = indent;
  if (!plugin_name_args_tab || htab_elements (plugin_name_args_tab) == 0)
    return;

  fprintf (file, "%sVersions of loaded plugins:\n", indent);
  htab_traverse_noresize (plugin_name_args_tab, print_version_one_plugin, &opt);
}


// Source: plugin.c
// Lines 840-850
