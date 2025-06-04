format_loop_sessions(struct format_expand_state *es, const char *fmt)
{
	struct format_tree		*ft = es->ft;
	struct client			*c = ft->client;
	struct cmdq_item		*item = ft->item;
	struct format_tree		*nft;
	struct format_expand_state	 next;
	char				*expanded, *value;
	size_t				 valuelen;
	struct session			*s;

	value = xcalloc(1, 1);
	valuelen = 1;

	RB_FOREACH(s, sessions, &sessions) {
		format_log(es, "session loop: $%u", s->id);
		nft = format_create(c, item, FORMAT_NONE, ft->flags);
		format_defaults(nft, ft->c, s, NULL, NULL);
		format_copy_state(&next, es, 0);
		next.ft = nft;
		expanded = format_expand1(&next, fmt);
		format_free(next.ft);

		valuelen += strlen(expanded);
		value = xrealloc(value, valuelen);

		strlcat(value, expanded, valuelen);
		free(expanded);
	}

	return (value);
}


// Source: format.c
// Lines 3740-3771
