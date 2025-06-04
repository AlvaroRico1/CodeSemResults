popup_resize_cb(__unused struct client *c, void *data)
{
	struct popup_data	*pd = data;
	struct tty		*tty = &c->tty;

	if (pd == NULL)
		return;
	if (pd->md != NULL)
		menu_free_cb(c, pd->md);

	/* Adjust position and size. */
	if (pd->psy > tty->sy)
		pd->sy = tty->sy;
	else
		pd->sy = pd->psy;
	if (pd->psx > tty->sx)
		pd->sx = tty->sx;
	else
		pd->sx = pd->psx;
	if (pd->ppy + pd->sy > tty->sy)
		pd->py = tty->sy - pd->sy;
	else
		pd->py = pd->ppy;
	if (pd->ppx + pd->sx > tty->sx)
		pd->px = tty->sx - pd->sx;
	else
		pd->px = pd->ppx;

	/* Avoid zero size screens. */
	if (pd->lines == BOX_LINES_NONE) {
		screen_resize(&pd->s, pd->sx, pd->sy, 0);
		if (pd->job != NULL)
			job_resize(pd->job, pd->sx, pd->sy );
	} else if (pd->sx > 2 && pd->sy > 2) {
		screen_resize(&pd->s, pd->sx - 2, pd->sy - 2, 0);
		if (pd->job != NULL)
			job_resize(pd->job, pd->sx - 2, pd->sy - 2);
	}
}


// Source: popup.c
// Lines 296-334
