cmdq_print(struct cmdq_item *item, const char *fmt, ...)
{
	struct client			*c = item->client;
	struct window_pane		*wp;
	struct window_mode_entry	*wme;
	va_list				 ap;
	char				*tmp, *msg;

	va_start(ap, fmt);
	xvasprintf(&msg, fmt, ap);
	va_end(ap);

	log_debug("%s: %s", __func__, msg);

	if (c == NULL)
		/* nothing */;
	else if (c->session == NULL || (c->flags & CLIENT_CONTROL)) {
		if (~c->flags & CLIENT_UTF8) {
			tmp = msg;
			msg = utf8_sanitize(tmp);
			free(tmp);
		}
		if (c->flags & CLIENT_CONTROL)
			control_write(c, "%s", msg);
		else
			file_print(c, "%s\n", msg);
	} else {
		wp = server_client_get_pane(c);
		wme = TAILQ_FIRST(&wp->modes);
		if (wme == NULL || wme->mode != &window_view_mode) {
			window_pane_set_mode(wp, NULL, &window_view_mode, NULL,
			    NULL);
		}
		window_copy_add(wp, "%s", msg);
	}

	free(msg);
}


// Source: cmd-queue.c
// Lines 810-847
