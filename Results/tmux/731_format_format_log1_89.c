format_log1(struct format_expand_state *es, const char *from, const char *fmt,
    ...)
{
	struct format_tree	*ft = es->ft;
	va_list			 ap;
	char			*s;
	static const char	 spaces[] = "          ";

	if (!format_logging(ft))
		return;

	va_start(ap, fmt);
	xvasprintf(&s, fmt, ap);
	va_end(ap);

	log_debug("%s: %s", from, s);
	if (ft->item != NULL && (ft->flags & FORMAT_VERBOSE))
		cmdq_print(ft->item, "#%.*s%s", es->loop, spaces, s);

	free(s);
}


// Source: format.c
// Lines 245-265
