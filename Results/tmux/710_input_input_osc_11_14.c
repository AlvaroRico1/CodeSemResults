input_osc_11(struct input_ctx *ictx, const char *p)
{
	struct window_pane	*wp = ictx->wp;
	struct grid_cell	 defaults;
	int			 c;

	if (strcmp(p, "?") == 0) {
		if (wp != NULL) {
			tty_default_colours(&defaults, wp);
			input_osc_colour_reply(ictx, 11, defaults.bg);
		}
		return;
	}

	if ((c = input_osc_parse_colour(p)) == -1) {
		log_debug("bad OSC 11: %s", p);
		return;
	}
	if (ictx->palette != NULL) {
		ictx->palette->bg = c;
		if (wp != NULL)
			wp->flags |= PANE_STYLECHANGED;
		screen_write_fullredraw(&ictx->ctx);
	}
}


// Source: input.c
// Lines 2585-2609
