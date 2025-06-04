format_cb_pane_at_bottom(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	struct window		*w;
	int			 status, flag;
	char			*value;

	if (wp == NULL)
		return (NULL);
	w = wp->window;

	status = options_get_number(w->options, "pane-border-status");
	if (status == PANE_STATUS_BOTTOM)
		flag = (wp->yoff + wp->sy == w->sy - 1);
	else
		flag = (wp->yoff + wp->sy == w->sy);
	xasprintf(&value, "%d", flag);
	return (value);
}


// Source: format.c
// Lines 1068-1086
