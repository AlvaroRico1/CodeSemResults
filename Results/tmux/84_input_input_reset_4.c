input_reset(struct input_ctx *ictx, int clear)
{
	struct screen_write_ctx	*sctx = &ictx->ctx;
	struct window_pane	*wp = ictx->wp;

	input_reset_cell(ictx);

	if (clear && wp != NULL) {
		if (TAILQ_EMPTY(&wp->modes))
			screen_write_start_pane(sctx, wp, &wp->base);
		else
			screen_write_start(sctx, &wp->base);
		screen_write_reset(sctx);
		screen_write_stop(sctx);
	}

	input_clear(ictx);

	ictx->last = -1;

	ictx->state = &input_state_ground;
	ictx->flags = 0;
}


// Source: input.c
// Lines 845-867
