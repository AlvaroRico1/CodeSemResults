static int diff_patchid_print_callback_to_buf(
	const git_diff_delta *delta,
	const git_diff_hunk *hunk,
	const git_diff_line *line,
	void *payload)
{
	struct patch_id_args *args = (struct patch_id_args *) payload;
	git_str buf = GIT_STR_INIT;
	int error = 0;

	if (line->origin == GIT_DIFF_LINE_CONTEXT_EOFNL ||
	    line->origin == GIT_DIFF_LINE_ADD_EOFNL ||
	    line->origin == GIT_DIFF_LINE_DEL_EOFNL)
		goto out;

	if ((error = git_diff_print_callback__to_buf(delta, hunk,
						     line, &buf)) < 0)
		goto out;

	strip_spaces(&buf);

	if (line->origin == GIT_DIFF_LINE_FILE_HDR &&
	    !args->first_file &&
	    (error = flush_hunk(&args->result, &args->ctx) < 0))
			goto out;

	if ((error = git_hash_update(&args->ctx, buf.ptr, buf.size)) < 0)
		goto out;

	if (line->origin == GIT_DIFF_LINE_FILE_HDR && args->first_file)
		args->first_file = 0;

out:
	git_str_dispose(&buf);
	return error;
}


// Source: diff.c
// Lines 318-353
