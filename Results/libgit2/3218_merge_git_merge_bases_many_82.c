int git_merge_bases_many(git_oidarray *out, git_repository *repo, size_t length, const git_oid input_array[])
{
	git_revwalk *walk;
	git_commit_list *list, *result = NULL;
	int error = 0;
	git_array_oid_t array;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(input_array);

	if ((error = merge_bases_many(&result, &walk, repo, length, input_array)) < 0)
		return error;

	git_array_init(array);

	list = result;
	while (list) {
		git_oid *id = git_array_alloc(array);
		if (id == NULL) {
			error = -1;
			goto cleanup;
		}

		git_oid_cpy(id, &list->item->oid);
		list = list->next;
	}

	git_oidarray__from_array(out, &array);

cleanup:
	git_commit_list_free(&result);
	git_revwalk_free(walk);

	return error;
}


// Source: merge.c
// Lines 157-192
