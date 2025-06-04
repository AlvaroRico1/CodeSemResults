static void find_subpos(const char *buf,
			const char **sub, size_t *sublen,
			const char **body, size_t *bodylen,
			size_t *nonsiglen,
			const char **sig, size_t *siglen)
{
	struct strbuf payload = STRBUF_INIT;
	struct strbuf signature = STRBUF_INIT;
	const char *eol;
	const char *end = buf + strlen(buf);
	const char *sigstart;

	/* parse signature first; we might not even have a subject line */
	parse_signature(buf, end - buf, &payload, &signature);

	/* skip past header until we hit empty line */
	while (*buf && *buf != '\n') {
		eol = strchrnul(buf, '\n');
		if (*eol)
			eol++;
		buf = eol;
	}


// Source: ref-filter.c
// Lines 1346-1367
