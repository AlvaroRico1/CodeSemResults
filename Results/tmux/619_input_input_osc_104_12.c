input_osc_104(struct input_ctx *ictx, const char *p)
{
	char	*copy, *s;
	long	 idx;
	int	 bad = 0, redraw = 0;

	if (*p == '\0') {
		colour_palette_clear(ictx->palette);
		screen_write_fullredraw(&ictx->ctx);
		return;
	}

	copy = s = xstrdup(p);
	while (*s != '\0') {
		idx = strtol(s, &s, 10);
		if (*s != '\0' && *s != ';') {
			bad = 1;
			break;
		}
		if (idx < 0 || idx >= 256) {
			bad = 1;
			break;
		}
		if (colour_palette_set(ictx->palette, idx, -1))
			redraw = 1;
		if (*s == ';')
			s++;
	}
	if (bad)
		log_debug("bad OSC 104: %s", p);
	if (redraw)
		screen_write_fullredraw(&ictx->ctx);
	free(copy);
}


// Source: input.c
// Lines 2697-2730
