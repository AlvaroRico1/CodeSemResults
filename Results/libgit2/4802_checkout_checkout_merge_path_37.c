static int checkout_merge_path(
	git_str *out,
	checkout_data *data,
	checkout_conflictdata *conflict,
	git_merge_file_result *result)
{
	const char *our_label_raw, *their_label_raw, *suffix;
	int error = 0;

	if ((error = git_str_joinpath(out, data->opts.target_directory, result->path)) < 0 ||
	    (error = git_path_validate_str_length(data->repo, out)) < 0)
		return error;

	/* Most conflicts simply use the filename in the index */
	if (!conflict->name_collision)
		return 0;

	/* Rename 2->1 conflicts need the branch name appended */
	our_label_raw = data->opts.our_label ? data->opts.our_label : "ours";
	their_label_raw = data->opts.their_label ? data->opts.their_label : "theirs";
	suffix = strcmp(result->path, conflict->ours->path) == 0 ? our_label_raw : their_label_raw;

	if ((error = checkout_path_suffixed(out, suffix)) < 0)
		return error;

	return 0;
}


// Source: checkout.c
// Lines 2028-2054
