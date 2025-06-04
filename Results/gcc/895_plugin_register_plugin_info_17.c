register_plugin_info (const char* name, struct plugin_info *info)
{
  void **slot = htab_find_slot_with_hash (plugin_name_args_tab, name,
					  htab_hash_string (name), NO_INSERT);
  struct plugin_name_args *plugin;

  if (slot == NULL)
    {
      error ("unable to register info for plugin %qs - plugin name not found",
	     name);
      return;
    }
  plugin = (struct plugin_name_args *) *slot;
  plugin->version = info->version;
  plugin->help = info->help;
}


// Source: plugin.c
// Lines 368-383
