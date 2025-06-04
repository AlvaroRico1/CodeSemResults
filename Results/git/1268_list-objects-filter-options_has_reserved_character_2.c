static int has_reserved_character(
	struct strbuf *sub_spec, struct strbuf *errbuf)
{
	const char *c = sub_spec->buf;
	while (*c) {
		if (*c <= ' ' || strchr(RESERVED_NON_WS, *c)) {
			strbuf_addf(
				errbuf,
				_("must escape char in sub-filter-spec: '%c'"),
				*c);
			return 1;
		}
		c++;
	}

	return 0;
}


// Source: list-objects-filter-options.c
// Lines 132-148
