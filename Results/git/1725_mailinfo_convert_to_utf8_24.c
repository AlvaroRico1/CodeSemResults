static int convert_to_utf8(struct mailinfo *mi,
			   struct strbuf *line, const char *charset)
{
	char *out;
	size_t out_len;

	if (!mi->metainfo_charset || !charset || !*charset)
		return 0;

	if (same_encoding(mi->metainfo_charset, charset))
		return 0;
	out = reencode_string_len(line->buf, line->len,
				  mi->metainfo_charset, charset, &out_len);
	if (!out) {
		mi->input_error = -1;
		return error("cannot convert from %s to %s",
			     charset, mi->metainfo_charset);
	}
	strbuf_attach(line, out, out_len, out_len);
	return 0;
}


// Source: mailinfo.c
// Lines 446-466
