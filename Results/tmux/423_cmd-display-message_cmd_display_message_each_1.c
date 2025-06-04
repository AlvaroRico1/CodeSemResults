cmd_display_message_each(const char *key, const char *value, void *arg)
{
	struct cmdq_item	*item = arg;

	cmdq_print(item, "%s=%s", key, value);
}


// Source: cmd-display-message.c
// Lines 53-58
