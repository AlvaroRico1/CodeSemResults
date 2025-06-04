popup_redraw_cb(const struct tty_ctx *ttyctx)
{
	struct popup_data	*pd = ttyctx->arg;

	pd->c->flags |= CLIENT_REDRAWOVERLAY;
}


// Source: popup.c
// Lines 98-103
