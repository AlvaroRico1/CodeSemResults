int git_merge_file_from_index(
	git_merge_file_result *out,
	git_repository *repo,
	const git_index_entry *ancestor,
	const git_index_entry *ours,
	const git_index_entry *theirs,
	const git_merge_file_options *options)
{
	git_merge_file_input *ancestor_ptr = NULL,
		ancestor_input = {0}, our_input = {0}, their_input = {0};
	git_odb *odb = NULL;
	git_odb_object *odb_object[3] = { 0 };
	int error = 0;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(ours);
	GIT_ASSERT_ARG(theirs);

	memset(out, 0x0, sizeof(git_merge_file_result));

	if ((error = git_repository_odb(&odb, repo)) < 0)
		goto done;

	if (ancestor) {
		if ((error = merge_file_input_from_index(
			&ancestor_input, &odb_object[0], odb, ancestor)) < 0)
			goto done;

		ancestor_ptr = &ancestor_input;
	}

	if ((error = merge_file_input_from_index(&our_input, &odb_object[1], odb, ours)) < 0 ||
	    (error = merge_file_input_from_index(&their_input, &odb_object[2], odb, theirs)) < 0)
		goto done;

	error = merge_file__from_inputs(out,
		ancestor_ptr, &our_input, &their_input, options);

done:
	git_odb_object_free(odb_object[0]);
	git_odb_object_free(odb_object[1]);
	git_odb_object_free(odb_object[2]);
	git_odb_free(odb);

	return error;
}


// Source: merge_file.c
// Lines 272-318
