format_cb_pane_dead_status(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;

	if (wp != NULL) {
		if ((wp->flags & PANE_STATUSREADY) && WIFEXITED(wp->status))
			return (format_printf("%d", WEXITSTATUS(wp->status)));
		return (NULL);
	}
	return (NULL);
}


// Source: format.c
// Lines 1723-1733
