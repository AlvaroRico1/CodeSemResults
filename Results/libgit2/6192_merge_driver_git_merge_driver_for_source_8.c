int git_merge_driver_for_source(
	const char **name_out,
	git_merge_driver **driver_out,
	const git_merge_driver_source *src)
{
	const char *path, *driver_name;
	int error = 0;

	path = git_merge_file__best_path(
		src->ancestor ? src->ancestor->path : NULL,
		src->ours ? src->ours->path : NULL,
		src->theirs ? src->theirs->path : NULL);

	if ((error = merge_driver_name_for_path(
			&driver_name, src->repo, path, src->default_driver)) < 0)
		return error;

	*name_out = driver_name;
	*driver_out = merge_driver_lookup_with_wildcard(driver_name);
	return error;
}


// Source: merge_driver.c
// Lines 411-431
