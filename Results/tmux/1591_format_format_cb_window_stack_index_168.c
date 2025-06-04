format_cb_window_stack_index(struct format_tree *ft)
{
	struct session	*s;
	struct winlink	*wl;
	u_int		 idx;
	char		*value = NULL;

	if (ft->wl == NULL)
		return (NULL);
	s = ft->wl->session;

	idx = 0;
	TAILQ_FOREACH(wl, &s->lastw, sentry) {
		idx++;
		if (wl == ft->wl)
			break;
	}
	if (wl == NULL)
		return (xstrdup("0"));
	xasprintf(&value, "%u", idx);
	return (value);
}


// Source: format.c
// Lines 593-614
