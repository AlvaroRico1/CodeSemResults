htab_str_eq (const void *s1, const void *s2)
{
  const struct plugin_name_args *plugin = (const struct plugin_name_args *) s1;
  return !strcmp (plugin->base_name, (const char *) s2);
}


// Source: plugin.c
// Lines 141-145
