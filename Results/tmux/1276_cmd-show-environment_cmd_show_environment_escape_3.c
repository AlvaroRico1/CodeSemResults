cmd_show_environment_escape(struct environ_entry *envent)
{
	const char	*value = envent->value;
	char		 c, *out, *ret;

	out = ret = xmalloc(strlen(value) * 2 + 1); /* at most twice the size */
	while ((c = *value++) != '\0') {
		/* POSIX interprets $ ` " and \ in double quotes. */
		if (c == '$' || c == '`' || c == '"' || c == '\\')
			*out++ = '\\';
		*out++ = c;
	}
	*out = '\0';

	return (ret);
}


// Source: cmd-show-environment.c
// Lines 51-66
