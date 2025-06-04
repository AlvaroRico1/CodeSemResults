window_tree_key(struct window_mode_entry *wme, struct client *c,
    __unused struct session *s, __unused struct winlink *wl, key_code key,
    struct mouse_event *m)
{
	struct window_pane		*wp = wme->wp;
	struct window_tree_modedata	*data = wme->data;
	struct window_tree_itemdata	*item, *new_item;
	char				*name, *prompt = NULL;
	struct cmd_find_state		 fs, *fsp = &data->fs;
	int				 finished;
	u_int				 tagged, x, y, idx;
	struct session			*ns;
	struct winlink			*nwl;
	struct window_pane		*nwp;

	item = mode_tree_get_current(data->data);
	finished = mode_tree_key(data->data, c, &key, m, &x, &y);
	if (item != (new_item = mode_tree_get_current(data->data))) {
		item = new_item;
		data->offset = 0;
	}
	if (KEYC_IS_MOUSE(key) && m != NULL)
		key = window_tree_mouse(data, key, x, item);
	switch (key) {
	case '<':
		data->offset--;
		break;
	case '>':
		data->offset++;
		break;
	case 'H':
		mode_tree_expand(data->data, (uint64_t)fsp->s);
		mode_tree_expand(data->data, (uint64_t)fsp->wl);
		if (!mode_tree_set_current(data->data, (uint64_t)wme->wp))
			mode_tree_set_current(data->data, (uint64_t)fsp->wl);
		break;
	case 'm':
		window_tree_pull_item(item, &ns, &nwl, &nwp);
		server_set_marked(ns, nwl, nwp);
		mode_tree_build(data->data);
		break;
	case 'M':
		server_clear_marked();
		mode_tree_build(data->data);
		break;
	case 'x':
		window_tree_pull_item(item, &ns, &nwl, &nwp);
		switch (item->type) {
		case WINDOW_TREE_NONE:
			break;
		case WINDOW_TREE_SESSION:
			if (ns == NULL)
				break;
			xasprintf(&prompt, "Kill session %s? ", ns->name);
			break;
		case WINDOW_TREE_WINDOW:
			if (nwl == NULL)
				break;
			xasprintf(&prompt, "Kill window %u? ", nwl->idx);
			break;
		case WINDOW_TREE_PANE:
			if (nwp == NULL || window_pane_index(nwp, &idx) != 0)
				break;
			xasprintf(&prompt, "Kill pane %u? ", idx);
			break;
		}
		if (prompt == NULL)
			break;
		data->references++;
		status_prompt_set(c, NULL, prompt, "",
		    window_tree_kill_current_callback, window_tree_command_free,
		    data, PROMPT_SINGLE|PROMPT_NOFORMAT, PROMPT_TYPE_COMMAND);
		free(prompt);
		break;
	case 'X':
		tagged = mode_tree_count_tagged(data->data);
		if (tagged == 0)
			break;
		xasprintf(&prompt, "Kill %u tagged? ", tagged);
		data->references++;
		status_prompt_set(c, NULL, prompt, "",
		    window_tree_kill_tagged_callback, window_tree_command_free,
		    data, PROMPT_SINGLE|PROMPT_NOFORMAT, PROMPT_TYPE_COMMAND);
		free(prompt);
		break;
	case ':':
		tagged = mode_tree_count_tagged(data->data);
		if (tagged != 0)
			xasprintf(&prompt, "(%u tagged) ", tagged);
		else
			xasprintf(&prompt, "(current) ");
		data->references++;
		status_prompt_set(c, NULL, prompt, "",
		    window_tree_command_callback, window_tree_command_free,
		    data, PROMPT_NOFORMAT, PROMPT_TYPE_COMMAND);
		free(prompt);
		break;
	case '\r':
		name = window_tree_get_target(item, &fs);
		if (name != NULL)
			mode_tree_run_command(c, NULL, data->command, name);
		finished = 1;
		free(name);
		break;
	}
	if (finished)
		window_pane_reset_mode(wp);
	else {
		mode_tree_draw(data->data);
		wp->flags |= PANE_REDRAW;
	}
}


// Source: window-tree.c
// Lines 1229-1340
