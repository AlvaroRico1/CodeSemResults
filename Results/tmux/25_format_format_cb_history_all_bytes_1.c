format_cb_history_all_bytes(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	struct grid		*gd;
	struct grid_line	*gl;
	u_int			 i, lines, cells = 0, extended_cells = 0;
	char			*value;

	if (wp == NULL)
		return (NULL);
	gd = wp->base.grid;

	lines = gd->hsize + gd->sy;
	for (i = 0; i < lines; i++) {
		gl = grid_get_line(gd, i);
		cells += gl->cellsize;
		extended_cells += gl->extdsize;
	}

	xasprintf(&value, "%u,%zu,%u,%zu,%u,%zu", lines,
	    lines * sizeof *gl, cells, cells * sizeof *gl->celldata,
	    extended_cells, extended_cells * sizeof *gl->extddata);
	return (value);
}


// Source: format.c
// Lines 870-893
