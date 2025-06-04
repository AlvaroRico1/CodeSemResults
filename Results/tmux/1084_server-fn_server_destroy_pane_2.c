server_destroy_pane(struct window_pane *wp, int notify)
{
	struct window		*w = wp->window;
	struct screen_write_ctx	 ctx;
	struct grid_cell	 gc;
	time_t			 t;
	char			 tim[26];
	int			 remain_on_exit;

	if (wp->fd != -1) {
#ifdef HAVE_UTEMPTER
		utempter_remove_record(wp->fd);
#endif
		bufferevent_free(wp->event);
		wp->event = NULL;
		close(wp->fd);
		wp->fd = -1;
	}

	remain_on_exit = options_get_number(wp->options, "remain-on-exit");
	if (remain_on_exit != 0 && (~wp->flags & PANE_STATUSREADY))
		return;
	switch (remain_on_exit) {
	case 0:
		break;
	case 2:
		if (WIFEXITED(wp->status) && WEXITSTATUS(wp->status) == 0)
			break;
		/* FALLTHROUGH */
	case 1:
		if (wp->flags & PANE_STATUSDRAWN)
			return;
		wp->flags |= PANE_STATUSDRAWN;

		if (notify)
			notify_pane("pane-died", wp);

		screen_write_start_pane(&ctx, wp, &wp->base);
		screen_write_scrollregion(&ctx, 0, screen_size_y(ctx.s) - 1);
		screen_write_cursormove(&ctx, 0, screen_size_y(ctx.s) - 1, 0);
		screen_write_linefeed(&ctx, 1, 8);
		memcpy(&gc, &grid_default_cell, sizeof gc);

		time(&t);
		ctime_r(&t, tim);
		tim[strcspn(tim, "\n")] = '\0';

		if (WIFEXITED(wp->status)) {
			screen_write_nputs(&ctx, -1, &gc,
			    "Pane is dead (status %d, %s)",
			    WEXITSTATUS(wp->status),
			    tim);
		} else if (WIFSIGNALED(wp->status)) {
			screen_write_nputs(&ctx, -1, &gc,
			    "Pane is dead (signal %s, %s)",
			    sig2name(WTERMSIG(wp->status)),
			    tim);
		}

		screen_write_stop(&ctx);
		wp->flags |= PANE_REDRAW;
		return;
	}

	if (notify)
		notify_pane("pane-exited", wp);

	server_unzoom_window(w);
	server_client_remove_pane(wp);
	layout_close_pane(wp);
	window_remove_pane(w, wp);

	if (TAILQ_EMPTY(&w->panes))
		server_kill_window(w, 1);
	else
		server_redraw_window(w);
}


// Source: server-fn.c
// Lines 308-384
