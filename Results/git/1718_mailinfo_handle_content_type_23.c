static void handle_content_type(struct mailinfo *mi, struct strbuf *line)
{
	struct strbuf *boundary = xmalloc(sizeof(struct strbuf));
	strbuf_init(boundary, line->len);

	mi->format_flowed = has_attr_value(line->buf, "format=", "flowed");
	mi->delsp = has_attr_value(line->buf, "delsp=", "yes");

	if (slurp_attr(line->buf, "boundary=", boundary)) {
		strbuf_insertstr(boundary, 0, "--");
		if (++mi->content_top >= &mi->content[MAX_BOUNDARIES]) {
			error("Too many boundaries to handle");
			mi->input_error = -1;
			mi->content_top = &mi->content[MAX_BOUNDARIES] - 1;
			return;
		}
		*(mi->content_top) = boundary;
		boundary = NULL;
	}
	slurp_attr(line->buf, "charset=", &mi->charset);

	if (boundary) {
		strbuf_release(boundary);
		free(boundary);
	}
}


// Source: mailinfo.c
// Lines 247-272
