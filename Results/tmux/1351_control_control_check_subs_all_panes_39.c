control_check_subs_all_panes(struct client *c, struct control_sub *csub)
{
	struct session		*s = c->session;
	struct window_pane	*wp;
	struct window		*w;
	struct winlink		*wl;
	struct format_tree	*ft;
	char			*value;
	struct control_sub_pane	*csp, find;

	RB_FOREACH(wl, winlinks, &s->windows) {
		w = wl->window;
		TAILQ_FOREACH(wp, &w->panes, entry) {
			ft = format_create_defaults(NULL, c, s, wl, wp);
			value = format_expand(ft, csub->format);
			format_free(ft);

			find.pane = wp->id;
			find.idx = wl->idx;

			csp = RB_FIND(control_sub_panes, &csub->panes, &find);
			if (csp == NULL) {
				csp = xcalloc(1, sizeof *csp);
				csp->pane = wp->id;
				csp->idx = wl->idx;
				RB_INSERT(control_sub_panes, &csub->panes, csp);
			}

			if (csp->last != NULL &&
			    strcmp(value, csp->last) == 0) {
				free(value);
				continue;
			}
			control_write(c,
			    "%%subscription-changed %s $%u @%u %u %%%u : %s",
			    csub->name, s->id, w->id, wl->idx, wp->id, value);
			free(csp->last);
			csp->last = value;
		}
	}
}


// Source: control.c
// Lines 904-944
