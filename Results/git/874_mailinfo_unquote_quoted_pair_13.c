static void unquote_quoted_pair(struct strbuf *line)
{
	struct strbuf outbuf;
	const char *in = line->buf;
	int c;

	strbuf_init(&outbuf, line->len);

	while ((c = *in++) != 0) {
		switch (c) {
		case '"':
			in = unquote_quoted_string(&outbuf, in);
			continue;
		case '(':
			in = unquote_comment(&outbuf, in);
			continue;
		}

		strbuf_addch(&outbuf, c);
	}

	strbuf_swap(&outbuf, line);
	strbuf_release(&outbuf);

}


// Source: mailinfo.c
// Lines 111-135
