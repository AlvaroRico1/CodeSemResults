cfg_show_causes(struct session *s)
{
	struct window_pane		*wp;
	struct window_mode_entry	*wme;
	u_int				 i;

	if (s == NULL || cfg_ncauses == 0)
		return;
	wp = s->curw->window->active;

	wme = TAILQ_FIRST(&wp->modes);
	if (wme == NULL || wme->mode != &window_view_mode)
		window_pane_set_mode(wp, NULL, &window_view_mode, NULL, NULL);
	for (i = 0; i < cfg_ncauses; i++) {
		window_copy_add(wp, "%s", cfg_causes[i]);
		free(cfg_causes[i]);
	}

	free(cfg_causes);
	cfg_causes = NULL;
	cfg_ncauses = 0;
}


// Source: cfg.c
// Lines 239-260
