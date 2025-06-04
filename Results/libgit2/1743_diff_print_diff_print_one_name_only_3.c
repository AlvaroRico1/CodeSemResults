static int diff_print_one_name_only(
	const git_diff_delta *delta, float progress, void *data)
{
	diff_print_info *pi = data;
	git_str *out = pi->buf;

	GIT_UNUSED(progress);

	if ((pi->flags & GIT_DIFF_SHOW_UNMODIFIED) == 0 &&
		delta->status == GIT_DELTA_UNMODIFIED)
		return 0;

	git_str_clear(out);
	git_str_puts(out, delta->new_file.path);
	git_str_putc(out, '\n');
	if (git_str_oom(out))
		return -1;

	pi->line.origin      = GIT_DIFF_LINE_FILE_HDR;
	pi->line.content     = git_str_cstr(out);
	pi->line.content_len = git_str_len(out);

	return pi->print_cb(delta, NULL, &pi->line, pi->payload);
}


// Source: diff_print.c
// Lines 142-165
