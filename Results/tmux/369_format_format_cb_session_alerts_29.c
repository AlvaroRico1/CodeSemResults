format_cb_session_alerts(struct format_tree *ft)
{
	struct session	*s = ft->s;
	struct winlink	*wl;
	char		 alerts[1024], tmp[16];

	if (s == NULL)
		return (NULL);

	*alerts = '\0';
	RB_FOREACH(wl, winlinks, &s->windows) {
		if ((wl->flags & WINLINK_ALERTFLAGS) == 0)
			continue;
		xsnprintf(tmp, sizeof tmp, "%u", wl->idx);

		if (*alerts != '\0')
			strlcat(alerts, ",", sizeof alerts);
		strlcat(alerts, tmp, sizeof alerts);
		if (wl->flags & WINLINK_ACTIVITY)
			strlcat(alerts, "#", sizeof alerts);
		if (wl->flags & WINLINK_BELL)
			strlcat(alerts, "!", sizeof alerts);
		if (wl->flags & WINLINK_SILENCE)
			strlcat(alerts, "~", sizeof alerts);
	}
	return (xstrdup(alerts));
}


// Source: format.c
// Lines 541-567
