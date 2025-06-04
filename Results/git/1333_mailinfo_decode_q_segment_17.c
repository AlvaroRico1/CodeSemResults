static struct strbuf *decode_q_segment(const struct strbuf *q_seg, int rfc2047)
{
	const char *in = q_seg->buf;
	int c;
	struct strbuf *out = xmalloc(sizeof(struct strbuf));
	strbuf_init(out, q_seg->len);

	while ((c = *in++) != 0) {
		if (c == '=') {
			int ch, d = *in;
			if (d == '\n' || !d)
				break; /* drop trailing newline */
			ch = hex2chr(in);
			if (ch >= 0) {
				strbuf_addch(out, ch);
				in += 2;
				continue;
			}
			/* garbage -- fall through */
		}
		if (rfc2047 && c == '_') /* rfc2047 4.2 (2) */
			c = 0x20;
		strbuf_addch(out, c);
	}
	return out;
}


// Source: mailinfo.c
// Lines 377-402
