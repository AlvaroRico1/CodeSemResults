cmd_find_get_pane_with_window(struct cmd_find_state *fs, const char *pane)
{
	const char		*errstr;
	int			 idx;
	struct window_pane	*wp;
	u_int			 n;

	log_debug("%s: %s", __func__, pane);

	/* Check for pane ids starting with %. */
	if (*pane == '%') {
		fs->wp = window_pane_find_by_id_str(pane);
		if (fs->wp == NULL)
			return (-1);
		if (fs->wp->window != fs->w)
			return (-1);
		return (0);
	}

	/* Try special characters. */
	if (strcmp(pane, "!") == 0) {
		fs->wp = fs->w->last;
		if (fs->wp == NULL)
			return (-1);
		return (0);
	} else if (strcmp(pane, "{up-of}") == 0) {
		fs->wp = window_pane_find_up(fs->current->wp);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	} else if (strcmp(pane, "{down-of}") == 0) {
		fs->wp = window_pane_find_down(fs->current->wp);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	} else if (strcmp(pane, "{left-of}") == 0) {
		fs->wp = window_pane_find_left(fs->current->wp);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	} else if (strcmp(pane, "{right-of}") == 0) {
		fs->wp = window_pane_find_right(fs->current->wp);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	}

	/* Try as an offset. */
	if (pane[0] == '+' || pane[0] == '-') {
		if (pane[1] != '\0')
			n = strtonum(pane + 1, 1, INT_MAX, NULL);
		else
			n = 1;
		wp = fs->current->wp;
		if (pane[0] == '+')
			fs->wp = window_pane_next_by_number(fs->w, wp, n);
		else
			fs->wp = window_pane_previous_by_number(fs->w, wp, n);
		if (fs->wp != NULL)
			return (0);
	}

	/* Get pane by index. */
	idx = strtonum(pane, 0, INT_MAX, &errstr);
	if (errstr == NULL) {
		fs->wp = window_pane_at_index(fs->w, idx);
		if (fs->wp != NULL)
			return (0);
	}

	/* Try as a description. */
	fs->wp = window_find_string(fs->w, pane);
	if (fs->wp != NULL)
		return (0);

	return (-1);
}


// Source: cmd-find.c
// Lines 564-640
