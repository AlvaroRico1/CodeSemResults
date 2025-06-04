static int is_multipart_boundary(struct mailinfo *mi, const struct strbuf *line)
{
	struct strbuf *content_top = *(mi->content_top);

	return ((content_top->len <= line->len) &&
		!memcmp(line->buf, content_top->buf, content_top->len));
}


// Source: mailinfo.c
// Lines 285-291
