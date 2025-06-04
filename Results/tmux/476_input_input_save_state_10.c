input_save_state(struct input_ctx *ictx)
{
	struct screen_write_ctx	*sctx = &ictx->ctx;
	struct screen		*s = sctx->s;

	memcpy(&ictx->old_cell, &ictx->cell, sizeof ictx->old_cell);
	ictx->old_cx = s->cx;
	ictx->old_cy = s->cy;
	ictx->old_mode = s->mode;
}


// Source: input.c
// Lines 774-783
