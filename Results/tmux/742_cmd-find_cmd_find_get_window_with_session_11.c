cmd_find_get_window_with_session(struct cmd_find_state *fs, const char *window)
{
	struct winlink	*wl;
	const char	*errstr;
	int		 idx, n, exact;
	struct session	*s;

	log_debug("%s: %s", __func__, window);
	exact = (fs->flags & CMD_FIND_EXACT_WINDOW);

	/*
	 * Start with the current window as the default. So if only an index is
	 * found, the window will be the current.
	 */
	fs->wl = fs->s->curw;
	fs->w = fs->wl->window;

	/* Check for window ids starting with @. */
	if (*window == '@') {
		fs->w = window_find_by_id_str(window);
		if (fs->w == NULL || !session_has(fs->s, fs->w))
			return (-1);
		return (cmd_find_best_winlink_with_window(fs));
	}

	/* Try as an offset. */
	if (!exact && (window[0] == '+' || window[0] == '-')) {
		if (window[1] != '\0')
			n = strtonum(window + 1, 1, INT_MAX, NULL);
		else
			n = 1;
		s = fs->s;
		if (fs->flags & CMD_FIND_WINDOW_INDEX) {
			if (window[0] == '+') {
				if (INT_MAX - s->curw->idx < n)
					return (-1);
				fs->idx = s->curw->idx + n;
			} else {
				if (n > s->curw->idx)
					return (-1);
				fs->idx = s->curw->idx - n;
			}
			return (0);
		}
		if (window[0] == '+')
			fs->wl = winlink_next_by_number(s->curw, s, n);
		else
			fs->wl = winlink_previous_by_number(s->curw, s, n);
		if (fs->wl != NULL) {
			fs->idx = fs->wl->idx;
			fs->w = fs->wl->window;
			return (0);
		}
	}

	/* Try special characters. */
	if (!exact) {
		if (strcmp(window, "!") == 0) {
			fs->wl = TAILQ_FIRST(&fs->s->lastw);
			if (fs->wl == NULL)
				return (-1);
			fs->idx = fs->wl->idx;
			fs->w = fs->wl->window;
			return (0);
		} else if (strcmp(window, "^") == 0) {
			fs->wl = RB_MIN(winlinks, &fs->s->windows);
			if (fs->wl == NULL)
				return (-1);
			fs->idx = fs->wl->idx;
			fs->w = fs->wl->window;
			return (0);
		} else if (strcmp(window, "$") == 0) {
			fs->wl = RB_MAX(winlinks, &fs->s->windows);
			if (fs->wl == NULL)
				return (-1);
			fs->idx = fs->wl->idx;
			fs->w = fs->wl->window;
			return (0);
		}
	}

	/* First see if this is a valid window index in this session. */
	if (window[0] != '+' && window[0] != '-') {
		idx = strtonum(window, 0, INT_MAX, &errstr);
		if (errstr == NULL) {
			fs->wl = winlink_find_by_index(&fs->s->windows, idx);
			if (fs->wl != NULL) {
				fs->idx = fs->wl->idx;
				fs->w = fs->wl->window;
				return (0);
			}
			if (fs->flags & CMD_FIND_WINDOW_INDEX) {
				fs->idx = idx;
				return (0);
			}
		}
	}

	/* Look for exact matches, error if more than one. */
	fs->wl = NULL;
	RB_FOREACH(wl, winlinks, &fs->s->windows) {
		if (strcmp(window, wl->window->name) == 0) {
			if (fs->wl != NULL)
				return (-1);
			fs->wl = wl;
		}
	}
	if (fs->wl != NULL) {
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	}

	/* Stop now if exact only. */
	if (exact)
		return (-1);

	/* Try as the start of a window name, error if multiple. */
	fs->wl = NULL;
	RB_FOREACH(wl, winlinks, &fs->s->windows) {
		if (strncmp(window, wl->window->name, strlen(window)) == 0) {
			if (fs->wl != NULL)
				return (-1);
			fs->wl = wl;
		}
	}
	if (fs->wl != NULL) {
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	}

	/* Now look for pattern matches, again error if multiple. */
	fs->wl = NULL;
	RB_FOREACH(wl, winlinks, &fs->s->windows) {
		if (fnmatch(window, wl->window->name, 0) == 0) {
			if (fs->wl != NULL)
				return (-1);
			fs->wl = wl;
		}
	}
	if (fs->wl != NULL) {
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	}

	return (-1);
}


// Source: cmd-find.c
// Lines 348-496
