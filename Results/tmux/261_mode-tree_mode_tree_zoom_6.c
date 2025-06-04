mode_tree_zoom(struct mode_tree_data *mtd, struct args *args)
{
	struct window_pane	*wp = mtd->wp;

	if (args_has(args, 'Z')) {
		mtd->zoomed = (wp->window->flags & WINDOW_ZOOMED);
		if (!mtd->zoomed && window_zoom(wp) == 0)
			server_redraw_window(wp->window);
	} else
		mtd->zoomed = -1;
}


// Source: mode-tree.c
// Lines 443-453
