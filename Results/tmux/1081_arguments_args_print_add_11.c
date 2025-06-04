args_print_add(char **buf, size_t *len, const char *fmt, ...)
{
	va_list	 ap;
	char	*s;
	size_t	 slen;

	va_start(ap, fmt);
	slen = xvasprintf(&s, fmt, ap);
	va_end(ap);

	*len += slen;
	*buf = xrealloc(*buf, *len);

	strlcat(*buf, s, *len);
	free(s);
}


// Source: arguments.c
// Lines 430-445
