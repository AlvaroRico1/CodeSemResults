htab_hash_plugin (const PTR p)
{
  const struct plugin_name_args *plugin = (const struct plugin_name_args *) p;
  return htab_hash_string (plugin->base_name);
 }


// Source: plugin.c
// Lines 131-135
