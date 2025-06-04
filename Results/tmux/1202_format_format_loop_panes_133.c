format_loop_panes(struct format_expand_state *es, const char *fmt)
{
	struct format_tree		*ft = es->ft;
	struct client			*c = ft->client;
	struct cmdq_item		*item = ft->item;
	struct format_tree		*nft;
	struct format_expand_state	 next;
	char				*all, *active, *use, *expanded, *value;
	size_t				 valuelen;
	struct window_pane		*wp;

	if (ft->w == NULL) {
		format_log(es, "pane loop but no window");
		return (NULL);
	}

	if (format_choose(es, fmt, &all, &active, 0) != 0) {
		all = xstrdup(fmt);
		active = NULL;
	}

	value = xcalloc(1, 1);
	valuelen = 1;

	TAILQ_FOREACH(wp, &ft->w->panes, entry) {
		format_log(es, "pane loop: %%%u", wp->id);
		if (active != NULL && wp == ft->w->active)
			use = active;
		else
			use = all;
		nft = format_create(c, item, FORMAT_PANE|wp->id, ft->flags);
		format_defaults(nft, ft->c, ft->s, ft->wl, wp);
		format_copy_state(&next, es, 0);
		next.ft = nft;
		expanded = format_expand1(&next, use);
		format_free(nft);

		valuelen += strlen(expanded);
		value = xrealloc(value, valuelen);

		strlcat(value, expanded, valuelen);
		free(expanded);
	}

	free(active);
	free(all);

	return (value);
}


// Source: format.c
// Lines 3853-3901
