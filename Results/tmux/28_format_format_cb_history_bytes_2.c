format_cb_history_bytes(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	struct grid		*gd;
	struct grid_line	*gl;
	size_t		         size = 0;
	u_int			 i;
	char			*value;

	if (wp == NULL)
		return (NULL);
	gd = wp->base.grid;

	for (i = 0; i < gd->hsize + gd->sy; i++) {
		gl = grid_get_line(gd, i);
		size += gl->cellsize * sizeof *gl->celldata;
		size += gl->extdsize * sizeof *gl->extddata;
	}
	size += (gd->hsize + gd->sy) * sizeof *gl;

	xasprintf(&value, "%zu", size);
	return (value);
}


// Source: format.c
// Lines 844-866
