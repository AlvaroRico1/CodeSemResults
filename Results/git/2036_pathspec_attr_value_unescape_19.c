static char *attr_value_unescape(const char *value)
{
	const char *src;
	char *dst, *ret;

	ret = xmallocz(strlen(value));
	for (src = value, dst = ret; *src; src++, dst++) {
		if (*src == '\\') {
			if (!src[1])
				die(_("Escape character '\\' not allowed as "
				      "last character in attr value"));
			src++;
		}
		if (invalid_value_char(*src))
			die("cannot use '%c' for value matching", *src);
		*dst = *src;
	}
	*dst = '\0';
	return ret;
}


// Source: pathspec.c
// Lines 141-160
