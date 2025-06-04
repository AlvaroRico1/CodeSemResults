static int parse_mailmap_entry(
	git_str *real_name, git_str *real_email,
	git_str *replace_name, git_str *replace_email,
	git_parse_ctx *ctx)
{
	const char *start;
	size_t len;

	git_str_clear(real_name);
	git_str_clear(real_email);
	git_str_clear(replace_name);
	git_str_clear(replace_email);

	git_parse_advance_ws(ctx);
	if (is_eol(ctx))
		return -1; /* blank line */

	/* Parse the real name */
	if (advance_until(&start, &len, ctx, '<') < 0)
		return -1;

	git_str_attach_notowned(real_name, start, len);
	git_str_rtrim(real_name);

	/*
	 * If this is the last email in the line, this is the email to replace,
	 * otherwise, it's the real email.
	 */
	if (advance_until(&start, &len, ctx, '>') < 0)
		return -1;

	/* If we aren't at the end of the line, parse a second name and email */
	if (!is_eol(ctx)) {
		git_str_attach_notowned(real_email, start, len);

		git_parse_advance_ws(ctx);
		if (advance_until(&start, &len, ctx, '<') < 0)
			return -1;
		git_str_attach_notowned(replace_name, start, len);
		git_str_rtrim(replace_name);

		if (advance_until(&start, &len, ctx, '>') < 0)
			return -1;
	}

	git_str_attach_notowned(replace_email, start, len);

	if (!is_eol(ctx))
		return -1;

	return 0;
}


// Source: mailmap.c
// Lines 98-149
