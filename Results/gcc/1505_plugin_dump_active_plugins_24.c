dump_active_plugins (FILE *file)
{
  int event;

  if (!plugins_active_p ())
    return;

  fprintf (file, FMT_FOR_PLUGIN_EVENT " | %s\n", _("Event"), _("Plugins"));
  for (event = PLUGIN_PASS_MANAGER_SETUP; event < event_last; event++)
    if (plugin_callbacks[event])
      {
	struct callback_info *ci;

	fprintf (file, FMT_FOR_PLUGIN_EVENT " |", plugin_event_name[event]);

	for (ci = plugin_callbacks[event]; ci; ci = ci->next)
	  fprintf (file, " %s", ci->plugin_name);

	putc ('\n', file);
      }


// Source: plugin.c
// Lines 917-936
