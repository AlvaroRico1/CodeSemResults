spawn_log(const char *from, struct spawn_context *sc)
{
	struct session		*s = sc->s;
	struct winlink		*wl = sc->wl;
	struct window_pane	*wp0 = sc->wp0;
	const char		*name = cmdq_get_name(sc->item);
	char			 tmp[128];

	log_debug("%s: %s, flags=%#x", from, name, sc->flags);

	if (wl != NULL && wp0 != NULL)
		xsnprintf(tmp, sizeof tmp, "wl=%d wp0=%%%u", wl->idx, wp0->id);
	else if (wl != NULL)
		xsnprintf(tmp, sizeof tmp, "wl=%d wp0=none", wl->idx);
	else if (wp0 != NULL)
		xsnprintf(tmp, sizeof tmp, "wl=none wp0=%%%u", wp0->id);
	else
		xsnprintf(tmp, sizeof tmp, "wl=none wp0=none");
	log_debug("%s: s=$%u %s idx=%d", from, s->id, tmp, sc->idx);
	log_debug("%s: name=%s", from, sc->name == NULL ? "none" : sc->name);
}


// Source: spawn.c
// Lines 52-72
