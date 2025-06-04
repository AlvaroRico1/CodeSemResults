static int apply_each_file(const git_diff_delta *delta, float progress, void *payload)
{
	struct foreach_diff_data *data = payload;
	const char *match, *path;
	int error = 0;

	GIT_UNUSED(progress);

	path = delta->old_file.path;

	/* We only want those which match the pathspecs */
	if (!git_pathspec__match(
		    &data->pathspec->pathspec, path, false, (bool)data->index->ignore_case,
		    &match, NULL))
		return 0;

	if (data->cb)
		error = data->cb(path, match, data->payload);

	if (error > 0) /* skip this entry */
		return 0;
	if (error < 0) /* actual error */
		return error;

	/* If the workdir item does not exist, remove it from the index. */
	if ((delta->new_file.flags & GIT_DIFF_FLAG_EXISTS) == 0)
		error = git_index_remove_bypath(data->index, path);
	else
		error = git_index_add_bypath(data->index, delta->new_file.path);

	return error;
}


// Source: index.c
// Lines 3438-3469
