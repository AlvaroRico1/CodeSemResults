input_osc_4(struct input_ctx *ictx, const char *p)
{
	char	*copy, *s, *next = NULL;
	long	 idx;
	int	 c, bad = 0, redraw = 0;

	copy = s = xstrdup(p);
	while (s != NULL && *s != '\0') {
		idx = strtol(s, &next, 10);
		if (*next++ != ';') {
			bad = 1;
			break;
		}
		if (idx < 0 || idx >= 256) {
			bad = 1;
			break;
		}

		s = strsep(&next, ";");
		if ((c = input_osc_parse_colour(s)) == -1) {
			s = next;
			continue;
		}
		if (colour_palette_set(ictx->palette, idx, c))
			redraw = 1;
		s = next;
	}
	if (bad)
		log_debug("bad OSC 4: %s", p);
	if (redraw)
		screen_write_fullredraw(&ictx->ctx);
	free(copy);
}


// Source: input.c
// Lines 2505-2537
