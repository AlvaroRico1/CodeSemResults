unregister_callback (const char *plugin_name, int event)
{
  struct callback_info *callback, **cbp;

  if (event >= event_last)
    return PLUGEVT_NO_SUCH_EVENT;

  for (cbp = &plugin_callbacks[event]; (callback = *cbp); cbp = &callback->next)
    if (strcmp (callback->plugin_name, plugin_name) == 0)
      {
	*cbp = callback->next;
	return PLUGEVT_SUCCESS;
      }
  return PLUGEVT_NO_CALLBACK;
}


// Source: plugin.c
// Lines 524-538
