static inline int skip_header(const struct strbuf *line, const char *hdr,
			      const char **outval)
{
	const char *val;
	if (!skip_iprefix(line->buf, hdr, &val) ||
	    *val++ != ':')
		return 0;
	while (isspace(*val))
		val++;
	*outval = val;
	return 1;
}


// Source: mailinfo.c
// Lines 348-359
