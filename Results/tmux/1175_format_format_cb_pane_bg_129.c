format_cb_pane_bg(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	struct grid_cell	 gc;

	if (wp == NULL)
		return (NULL);

	tty_default_colours(&gc, wp);
	return (xstrdup(colour_tostring(gc.bg)));
}


// Source: format.c
// Lines 941-951
