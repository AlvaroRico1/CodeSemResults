int git_merge_bases(git_oidarray *out, git_repository *repo, const git_oid *one, const git_oid *two)
{
	int error;
	git_revwalk *walk;
	git_commit_list *result, *list;
	git_array_oid_t array;

	git_array_init(array);

	if ((error = merge_bases(&result, &walk, repo, one, two)) < 0)
		return error;

	list = result;
	while (list) {
		git_oid *id = git_array_alloc(array);
		if (id == NULL)
			goto on_error;

		git_oid_cpy(id, &list->item->oid);
		list = list->next;
	}

	git_oidarray__from_array(out, &array);
	git_commit_list_free(&result);
	git_revwalk_free(walk);

	return 0;

on_error:
	git_commit_list_free(&result);
	git_revwalk_free(walk);
	return -1;
}


// Source: merge.c
// Lines 282-314
