cmd_template_replace(const char *template, const char *s, int idx)
{
	char		 ch, *buf;
	const char	*ptr, *cp, quote[] = "\"\\$;~";
	int		 replaced, quoted;
	size_t		 len;

	if (strchr(template, '%') == NULL)
		return (xstrdup(template));

	buf = xmalloc(1);
	*buf = '\0';
	len = 0;
	replaced = 0;

	ptr = template;
	while (*ptr != '\0') {
		switch (ch = *ptr++) {
		case '%':
			if (*ptr < '1' || *ptr > '9' || *ptr - '0' != idx) {
				if (*ptr != '%' || replaced)
					break;
				replaced = 1;
			}
			ptr++;

			quoted = (*ptr == '%');
			if (quoted)
				ptr++;

			buf = xrealloc(buf, len + (strlen(s) * 3) + 1);
			for (cp = s; *cp != '\0'; cp++) {
				if (quoted && strchr(quote, *cp) != NULL)
					buf[len++] = '\\';
				buf[len++] = *cp;
			}
			buf[len] = '\0';
			continue;
		}
		buf = xrealloc(buf, len + 2);
		buf[len++] = ch;
		buf[len] = '\0';
	}

	return (buf);
}


// Source: cmd.c
// Lines 825-870
