popup_mode_cb(__unused struct client *c, void *data, u_int *cx, u_int *cy)
{
	struct popup_data	*pd = data;

	if (pd->md != NULL)
		return (menu_mode_cb(c, pd->md, cx, cy));

	if (pd->lines == BOX_LINES_NONE) {
		*cx = pd->px + pd->s.cx;
		*cy = pd->py + pd->s.cy;
	} else {
		*cx = pd->px + 1 + pd->s.cx;
		*cy = pd->py + 1 + pd->s.cy;
	}
	return (&pd->s);
}


// Source: popup.c
// Lines 144-159
