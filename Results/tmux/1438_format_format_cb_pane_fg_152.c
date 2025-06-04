format_cb_pane_fg(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	struct grid_cell	 gc;

	if (wp == NULL)
		return (NULL);

	tty_default_colours(&gc, wp);
	return (xstrdup(colour_tostring(gc.fg)));
}


// Source: format.c
// Lines 927-937
