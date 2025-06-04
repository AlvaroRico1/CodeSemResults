format_cb_cursor_character(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	struct grid_cell	 gc;
	char			*value = NULL;

	if (wp == NULL)
		return (NULL);

	grid_view_get_cell(wp->base.grid, wp->base.cx, wp->base.cy, &gc);
	if (~gc.flags & GRID_FLAG_PADDING)
		xasprintf(&value, "%.*s", (int)gc.data.size, gc.data.data);
	return (value);
}


// Source: format.c
// Lines 1090-1103
