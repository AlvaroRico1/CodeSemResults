static void strip_spaces(git_str *buf)
{
	char *src = buf->ptr, *dst = buf->ptr;
	char c;
	size_t len = 0;

	while ((c = *src++) != '\0') {
		if (!git__isspace(c)) {
			*dst++ = c;
			len++;
		}
	}

	git_str_truncate(buf, len);
}


// Source: diff.c
// Lines 302-316
