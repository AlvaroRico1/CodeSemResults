int subprocess_read_status(int fd, struct strbuf *status)
{
	struct strbuf **pair;
	char *line;
	int len;

	for (;;) {
		len = packet_read_line_gently(fd, NULL, &line);
		if ((len < 0) || !line)
			break;
		pair = strbuf_split_str(line, '=', 2);
		if (pair[0] && pair[0]->len && pair[1]) {
			/* the last "status=<foo>" line wins */
			if (!strcmp(pair[0]->buf, "status=")) {
				strbuf_reset(status);
				strbuf_addbuf(status, pair[1]);
			}
		}
		strbuf_list_free(pair);
	}

	return (len < 0) ? len : 0;
}


// Source: sub-process.c
// Lines 30-52
