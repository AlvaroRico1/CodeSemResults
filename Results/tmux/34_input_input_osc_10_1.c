input_osc_10(struct input_ctx *ictx, const char *p)
{
	struct window_pane	*wp = ictx->wp;
	struct grid_cell	 defaults;
	int			 c;

	if (strcmp(p, "?") == 0) {
		if (wp != NULL) {
			tty_default_colours(&defaults, wp);
			input_osc_colour_reply(ictx, 10, defaults.fg);
		}
		return;
	}

	if ((c = input_osc_parse_colour(p)) == -1) {
		log_debug("bad OSC 10: %s", p);
		return;
	}
	if (ictx->palette != NULL) {
		ictx->palette->fg = c;
		if (wp != NULL)
			wp->flags |= PANE_STYLECHANGED;
		screen_write_fullredraw(&ictx->ctx);
	}
}


// Source: input.c
// Lines 2541-2565
