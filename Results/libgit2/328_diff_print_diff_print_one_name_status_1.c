static int diff_print_one_name_status(
	const git_diff_delta *delta, float progress, void *data)
{
	diff_print_info *pi = data;
	git_str *out = pi->buf;
	char old_suffix, new_suffix, code = git_diff_status_char(delta->status);
	int(*strcomp)(const char *, const char *) = pi->strcomp ?
		pi->strcomp : git__strcmp;

	GIT_UNUSED(progress);

	if ((pi->flags & GIT_DIFF_SHOW_UNMODIFIED) == 0 && code == ' ')
		return 0;

	old_suffix = diff_pick_suffix(delta->old_file.mode);
	new_suffix = diff_pick_suffix(delta->new_file.mode);

	git_str_clear(out);

	if (delta->old_file.path != delta->new_file.path &&
		strcomp(delta->old_file.path,delta->new_file.path) != 0)
		git_str_printf(out, "%c\t%s%c %s%c\n", code,
			delta->old_file.path, old_suffix, delta->new_file.path, new_suffix);
	else if (delta->old_file.mode != delta->new_file.mode &&
		delta->old_file.mode != 0 && delta->new_file.mode != 0)
		git_str_printf(out, "%c\t%s%c %s%c\n", code,
			delta->old_file.path, old_suffix, delta->new_file.path, new_suffix);
	else if (old_suffix != ' ')
		git_str_printf(out, "%c\t%s%c\n", code, delta->old_file.path, old_suffix);
	else
		git_str_printf(out, "%c\t%s\n", code, delta->old_file.path);
	if (git_str_oom(out))
		return -1;

	pi->line.origin      = GIT_DIFF_LINE_FILE_HDR;
	pi->line.content     = git_str_cstr(out);
	pi->line.content_len = git_str_len(out);

	return pi->print_cb(delta, NULL, &pi->line, pi->payload);
}


// Source: diff_print.c
// Lines 167-206
