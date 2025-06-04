format_cb_current_path(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	char			*cwd;

	if (wp == NULL)
		return (NULL);

	cwd = osdep_get_cwd(wp->fd);
	if (cwd == NULL)
		return (NULL);
	return (xstrdup(cwd));
}


// Source: format.c
// Lines 828-840
