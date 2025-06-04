finalize_one_plugin (void **slot, void * ARG_UNUSED (info))
{
  struct plugin_name_args *plugin = (struct plugin_name_args *) *slot;
  XDELETE (plugin);
  return 1;
}


// Source: plugin.c
// Lines 789-794
