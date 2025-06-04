static int file_cb(
	const git_diff_delta *delta,
	float progress,
	void *payload)
{
	struct diff_data *diff_data = payload;

	GIT_UNUSED(progress);

	if (delta->old_file.path)
		diff_data->old_path = git__strdup(delta->old_file.path);

	if (delta->new_file.path)
		diff_data->new_path = git__strdup(delta->new_file.path);

	git_oid_cpy(&diff_data->old_id, &delta->old_file.id);
	git_oid_cpy(&diff_data->new_id, &delta->new_file.id);

	return 0;
}


// Source: binary.c
// Lines 424-443
