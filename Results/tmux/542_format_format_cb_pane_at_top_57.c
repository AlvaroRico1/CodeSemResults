format_cb_pane_at_top(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	struct window		*w;
	int			 status, flag;
	char			*value;

	if (wp == NULL)
		return (NULL);
	w = wp->window;

	status = options_get_number(w->options, "pane-border-status");
	if (status == PANE_STATUS_TOP)
		flag = (wp->yoff == 1);
	else
		flag = (wp->yoff == 0);
	xasprintf(&value, "%d", flag);
	return (value);
}


// Source: format.c
// Lines 1046-1064
