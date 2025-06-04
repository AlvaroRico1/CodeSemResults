void fmt_output_subject(struct strbuf *filename,
			const char *subject,
			struct rev_info *info)
{
	const char *suffix = info->patch_suffix;
	int nr = info->nr;
	int start_len = filename->len;
	int max_len = start_len + info->patch_name_max - (strlen(suffix) + 1);

	if (info->reroll_count) {
		struct strbuf temp = STRBUF_INIT;

		strbuf_addf(&temp, "v%s", info->reroll_count);
		format_sanitized_subject(filename, temp.buf, temp.len);
		strbuf_addstr(filename, "-");
		strbuf_release(&temp);
	}
	strbuf_addf(filename, "%04d-%s", nr, subject);

	if (max_len < filename->len)
		strbuf_setlen(filename, max_len);
	strbuf_addstr(filename, suffix);
}


// Source: log-tree.c
// Lines 365-387
