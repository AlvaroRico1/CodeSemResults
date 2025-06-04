input_init(struct window_pane *wp, struct bufferevent *bev,
    struct colour_palette *palette)
{
	struct input_ctx	*ictx;

	ictx = xcalloc(1, sizeof *ictx);
	ictx->wp = wp;
	ictx->event = bev;
	ictx->palette = palette;

	ictx->input_space = INPUT_BUF_START;
	ictx->input_buf = xmalloc(INPUT_BUF_START);

	ictx->since_ground = evbuffer_new();
	if (ictx->since_ground == NULL)
		fatalx("out of memory");

	evtimer_set(&ictx->timer, input_timer_callback, ictx);

	input_reset(ictx, 0);
	return (ictx);
}


// Source: input.c
// Lines 801-822
