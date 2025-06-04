format_expand(struct format_tree *ft, const char *fmt)
{
	struct format_expand_state	es;

	memset(&es, 0, sizeof es);
	es.ft = ft;
	es.flags = 0;
	return (format_expand1(&es, fmt));
}


// Source: format.c
// Lines 4639-4647
