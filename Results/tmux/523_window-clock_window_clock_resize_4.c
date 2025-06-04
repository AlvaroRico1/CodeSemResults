window_clock_resize(struct window_mode_entry *wme, u_int sx, u_int sy)
{
	struct window_clock_mode_data	*data = wme->data;
	struct screen			*s = &data->screen;

	screen_resize(s, sx, sy, 0);
	window_clock_draw_screen(wme);
}


// Source: window-clock.c
// Lines 188-195
