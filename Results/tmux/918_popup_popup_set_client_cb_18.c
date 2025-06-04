popup_set_client_cb(struct tty_ctx *ttyctx, struct client *c)
{
	struct popup_data	*pd = ttyctx->arg;

	if (c != pd->c)
		return (0);
	if (pd->c->flags & CLIENT_REDRAWOVERLAY)
		return (0);

	ttyctx->bigger = 0;
	ttyctx->wox = 0;
	ttyctx->woy = 0;
	ttyctx->wsx = c->tty.sx;
	ttyctx->wsy = c->tty.sy;

	if (pd->lines == BOX_LINES_NONE) {
		ttyctx->xoff = ttyctx->rxoff = pd->px;
		ttyctx->yoff = ttyctx->ryoff = pd->py;
	} else {
		ttyctx->xoff = ttyctx->rxoff = pd->px + 1;
		ttyctx->yoff = ttyctx->ryoff = pd->py + 1;
	}

	return (1);
}


// Source: popup.c
// Lines 106-130
