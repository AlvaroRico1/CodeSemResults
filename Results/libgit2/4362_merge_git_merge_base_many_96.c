int git_merge_base_many(git_oid *out, git_repository *repo, size_t length, const git_oid input_array[])
{
	git_revwalk *walk;
	git_commit_list *result = NULL;
	int error = 0;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(input_array);

	if ((error = merge_bases_many(&result, &walk, repo, length, input_array)) < 0)
		return error;

	git_oid_cpy(out, &result->item->oid);

	git_commit_list_free(&result);
	git_revwalk_free(walk);

	return 0;
}


// Source: merge.c
// Lines 136-155
