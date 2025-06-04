format_cb_pane_in_mode(struct format_tree *ft)
{
	struct window_pane		*wp = ft->wp;
	u_int				 n = 0;
	struct window_mode_entry	*wme;
	char				*value;

	if (wp == NULL)
		return (NULL);

	TAILQ_FOREACH(wme, &wp->modes, entry)
		n++;
	xasprintf(&value, "%u", n);
	return (value);
}


// Source: format.c
// Lines 1028-1042
