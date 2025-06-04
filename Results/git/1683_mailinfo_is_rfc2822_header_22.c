static int is_rfc2822_header(const struct strbuf *line)
{
	/*
	 * The section that defines the loosest possible
	 * field name is "3.6.8 Optional fields".
	 *
	 * optional-field = field-name ":" unstructured CRLF
	 * field-name = 1*ftext
	 * ftext = %d33-57 / %59-126
	 */
	int ch;
	char *cp = line->buf;

	/* Count mbox From headers as headers */
	if (starts_with(cp, "From ") || starts_with(cp, ">From "))
		return 1;

	while ((ch = *cp++)) {
		if (ch == ':')
			return 1;
		if ((33 <= ch && ch <= 57) ||
		    (59 <= ch && ch <= 126))
			continue;
		break;
	}
	return 0;
}


// Source: mailinfo.c
// Lines 860-886
