screen_write_mode_clear(struct screen_write_ctx *ctx, int mode)
{
	struct screen	*s = ctx->s;

	s->mode &= ~mode;

	if (log_get_level() != 0)
		log_debug("%s: %s", __func__, screen_mode_to_string(mode));
}


// Source: screen-write.c
// Lines 843-851
