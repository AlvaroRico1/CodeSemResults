window_copy_get_line(struct window_pane *wp, u_int y)
{
	struct window_mode_entry	*wme = TAILQ_FIRST(&wp->modes);
	struct window_copy_mode_data	*data = wme->data;
	struct grid			*gd = data->screen.grid;

	return (format_grid_line(gd, gd->hsize + y));
}


// Source: window-copy.c
// Lines 708-715
