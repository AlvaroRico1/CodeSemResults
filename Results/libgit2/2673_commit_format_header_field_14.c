static int format_header_field(git_str *out, const char *field, const char *content)
{
	const char *lf;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(field);
	GIT_ASSERT_ARG(content);

	git_str_puts(out, field);
	git_str_putc(out, ' ');

	while ((lf = strchr(content, '\n')) != NULL) {
		git_str_put(out, content, lf - content);
		git_str_puts(out, "\n ");
		content = lf + 1;
	}

	git_str_puts(out, content);
	git_str_putc(out, '\n');

	return git_str_oom(out) ? -1 : 0;
}


// Source: commit.c
// Lines 937-958
