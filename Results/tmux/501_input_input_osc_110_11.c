input_osc_110(struct input_ctx *ictx, const char *p)
{
	struct window_pane	*wp = ictx->wp;

	if (*p != '\0')
		return;
	if (ictx->palette != NULL) {
		ictx->palette->fg = 8;
		if (wp != NULL)
			wp->flags |= PANE_STYLECHANGED;
		screen_write_fullredraw(&ictx->ctx);
	}
}


// Source: input.c
// Lines 2569-2581
