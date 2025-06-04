cmd_find_best_winlink_with_window(struct cmd_find_state *fs)
{
	struct winlink	 *wl, *wl_loop;

	log_debug("%s: window is @%u", __func__, fs->w->id);

	wl = NULL;
	if (fs->s->curw != NULL && fs->s->curw->window == fs->w)
		wl = fs->s->curw;
	else {
		RB_FOREACH(wl_loop, winlinks, &fs->s->windows) {
			if (wl_loop->window == fs->w) {
				wl = wl_loop;
				break;
			}
		}
	}
	if (wl == NULL)
		return (-1);
	fs->wl = wl;
	fs->idx = fs->wl->idx;
	return (0);
}


// Source: cmd-find.c
// Lines 209-231
