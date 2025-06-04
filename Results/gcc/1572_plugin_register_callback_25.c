register_callback (const char *plugin_name,
		   int event,
                   plugin_callback_func callback,
                   void *user_data)
{
  switch (event)
    {
      case PLUGIN_PASS_MANAGER_SETUP:
	gcc_assert (!callback);
        register_pass ((struct register_pass_info *) user_data);
        break;
      case PLUGIN_INFO:
	gcc_assert (!callback);
	register_plugin_info (plugin_name, (struct plugin_info *) user_data);
	break;
      case PLUGIN_REGISTER_GGC_ROOTS:
	gcc_assert (!callback);
        ggc_register_root_tab ((const struct ggc_root_tab*) user_data);
	break;
      case PLUGIN_EVENT_FIRST_DYNAMIC:
      default:
	if (event < PLUGIN_EVENT_FIRST_DYNAMIC || event >= event_last)
	  {
	    error ("unknown callback event registered by plugin %s",
		   plugin_name);
	    return;
	  }
      /* Fall through.  */
      case PLUGIN_START_PARSE_FUNCTION:
      case PLUGIN_FINISH_PARSE_FUNCTION:
      case PLUGIN_FINISH_TYPE:
      case PLUGIN_FINISH_DECL:
      case PLUGIN_START_UNIT:
      case PLUGIN_FINISH_UNIT:
      case PLUGIN_PRE_GENERICIZE:
      case PLUGIN_GGC_START:
      case PLUGIN_GGC_MARKING:
      case PLUGIN_GGC_END:
      case PLUGIN_ATTRIBUTES:
      case PLUGIN_PRAGMAS:
      case PLUGIN_FINISH:
      case PLUGIN_ALL_PASSES_START:
      case PLUGIN_ALL_PASSES_END:
      case PLUGIN_ALL_IPA_PASSES_START:
      case PLUGIN_ALL_IPA_PASSES_END:
      case PLUGIN_OVERRIDE_GATE:
      case PLUGIN_PASS_EXECUTION:
      case PLUGIN_EARLY_GIMPLE_PASSES_START:
      case PLUGIN_EARLY_GIMPLE_PASSES_END:
      case PLUGIN_NEW_PASS:
      case PLUGIN_INCLUDE_FILE:
        {
          struct callback_info *new_callback;
          if (!callback)
            {
              error ("plugin %s registered a null callback function "
		     "for event %s", plugin_name, plugin_event_name[event]);
              return;
            }
          new_callback = XNEW (struct callback_info);
          new_callback->plugin_name = plugin_name;
          new_callback->func = callback;
          new_callback->user_data = user_data;
          new_callback->next = plugin_callbacks[event];
          plugin_callbacks[event] = new_callback;
        }


// Source: plugin.c
// Lines 449-514
