format_cb_session_stack(struct format_tree *ft)
{
	struct session	*s = ft->s;
	struct winlink	*wl;
	char		 result[1024], tmp[16];

	if (s == NULL)
		return (NULL);

	xsnprintf(result, sizeof result, "%u", s->curw->idx);
	TAILQ_FOREACH(wl, &s->lastw, sentry) {
		xsnprintf(tmp, sizeof tmp, "%u", wl->idx);

		if (*result != '\0')
			strlcat(result, ",", sizeof result);
		strlcat(result, tmp, sizeof result);
	}
	return (xstrdup(result));
}


// Source: format.c
// Lines 571-589
