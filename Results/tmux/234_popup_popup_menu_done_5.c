popup_menu_done(__unused struct menu *menu, __unused u_int choice,
    key_code key, void *data)
{
	struct popup_data	*pd = data;
	struct client		*c = pd->c;
	struct paste_buffer	*pb;
	const char		*buf;
	size_t			 len;

	pd->md = NULL;
	pd->menu = NULL;
	server_redraw_client(pd->c);

	switch (key) {
	case 'p':
		pb = paste_get_top(NULL);
		if (pb != NULL) {
			buf = paste_buffer_data(pb, &len);
			bufferevent_write(job_get_event(pd->job), buf, len);
		}
		break;
	case 'F':
		pd->sx = c->tty.sx;
		pd->sy = c->tty.sy;
		pd->px = 0;
		pd->py = 0;
		server_redraw_client(c);
		break;
	case 'C':
		pd->px = c->tty.sx / 2 - pd->sx / 2;
		pd->py = c->tty.sy / 2 - pd->sy / 2;
		server_redraw_client(c);
		break;
	case 'h':
		popup_make_pane(pd, LAYOUT_LEFTRIGHT);
		break;
	case 'v':
		popup_make_pane(pd, LAYOUT_TOPBOTTOM);
		break;
	case 'q':
		pd->close = 1;
		break;
	}
}


// Source: popup.c
// Lines 377-420
