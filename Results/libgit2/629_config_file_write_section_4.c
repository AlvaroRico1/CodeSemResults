static int write_section(git_str *fbuf, const char *key)
{
	int result;
	const char *dot;
	git_str buf = GIT_STR_INIT;

	/* All of this just for [section "subsection"] */
	dot = strchr(key, '.');
	git_str_putc(&buf, '[');
	if (dot == NULL) {
		git_str_puts(&buf, key);
	} else {
		char *escaped;
		git_str_put(&buf, key, dot - key);
		escaped = escape_value(dot + 1);
		GIT_ERROR_CHECK_ALLOC(escaped);
		git_str_printf(&buf, " \"%s\"", escaped);
		git__free(escaped);
	}
	git_str_puts(&buf, "]\n");

	if (git_str_oom(&buf))
		return -1;

	result = git_str_put(fbuf, git_str_cstr(&buf), buf.size);
	git_str_dispose(&buf);

	return result;
}


// Source: config_file.c
// Lines 896-924
