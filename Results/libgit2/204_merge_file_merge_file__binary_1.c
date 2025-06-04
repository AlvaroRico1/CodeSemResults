static int merge_file__binary(
	git_merge_file_result *out,
	const git_merge_file_input *ours,
	const git_merge_file_input *theirs,
	const git_merge_file_options *given_opts)
{
	const git_merge_file_input *favored = NULL;

	memset(out, 0x0, sizeof(git_merge_file_result));

	if (given_opts && given_opts->favor == GIT_MERGE_FILE_FAVOR_OURS)
		favored = ours;
	else if (given_opts && given_opts->favor == GIT_MERGE_FILE_FAVOR_THEIRS)
		favored = theirs;
	else
		goto done;

	if ((out->path = git__strdup(favored->path)) == NULL ||
		(out->ptr = git__malloc(favored->size)) == NULL)
		goto done;

	memcpy((char *)out->ptr, favored->ptr, favored->size);
	out->len = favored->size;
	out->mode = favored->mode;
	out->automergeable = 1;

done:
	return 0;
}


// Source: merge_file.c
// Lines 188-216
