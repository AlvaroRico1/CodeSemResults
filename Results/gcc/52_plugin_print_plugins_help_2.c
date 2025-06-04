print_plugins_help (FILE *file, const char *indent)
{
  struct print_options opt;
  opt.file = file;
  opt.indent = indent;
  if (!plugin_name_args_tab || htab_elements (plugin_name_args_tab) == 0)
    return;

  fprintf (file, "%sHelp for the loaded plugins:\n", indent);
  htab_traverse_noresize (plugin_name_args_tab, print_help_one_plugin, &opt);
}


// Source: plugin.c
// Lines 885-895
