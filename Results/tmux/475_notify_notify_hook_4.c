notify_hook(struct cmdq_item *item, const char *name)
{
	struct cmd_find_state	*target = cmdq_get_target(item);
	struct notify_entry	 ne;

	memset(&ne, 0, sizeof ne);

	ne.name = name;
	cmd_find_copy_state(&ne.fs, target);

	ne.client = cmdq_get_client(item);
	ne.session = target->s;
	ne.window = target->w;
	ne.pane = (target->wp != NULL ? target->wp->id : -1);

	ne.formats = format_create(NULL, NULL, 0, FORMAT_NOJOBS);
	format_add(ne.formats, "hook", "%s", name);
	format_log_debug(ne.formats, __func__);

	notify_insert_hook(item, &ne);
	format_free(ne.formats);
}


// Source: notify.c
// Lines 189-210
