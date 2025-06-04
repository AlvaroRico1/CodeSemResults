mode_tree_filter_callback(__unused struct client *c, void *data, const char *s,
    __unused int done)
{
	struct mode_tree_data	*mtd = data;

	if (mtd->dead)
		return (0);

	if (mtd->filter != NULL)
		free(mtd->filter);
	if (s == NULL || *s == '\0')
		mtd->filter = NULL;
	else
		mtd->filter = xstrdup(s);

	mode_tree_build(mtd);
	mode_tree_draw(mtd);
	mtd->wp->flags |= PANE_REDRAW;

	return (0);
}


// Source: mode-tree.c
// Lines 880-900
