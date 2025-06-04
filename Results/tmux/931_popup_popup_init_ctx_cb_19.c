popup_init_ctx_cb(struct screen_write_ctx *ctx, struct tty_ctx *ttyctx)
{
	struct popup_data	*pd = ctx->arg;

	ttyctx->palette = &pd->palette;
	ttyctx->redraw_cb = popup_redraw_cb;
	ttyctx->set_client_cb = popup_set_client_cb;
	ttyctx->arg = pd;
}


// Source: popup.c
// Lines 133-141
