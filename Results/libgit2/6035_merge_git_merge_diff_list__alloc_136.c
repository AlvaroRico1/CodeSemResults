git_merge_diff_list *git_merge_diff_list__alloc(git_repository *repo)
{
	git_merge_diff_list *diff_list = git__calloc(1, sizeof(git_merge_diff_list));

	if (diff_list == NULL)
		return NULL;

	diff_list->repo = repo;


	if (git_pool_init(&diff_list->pool, 1) < 0 ||
	    git_vector_init(&diff_list->staged, 0, NULL) < 0 ||
	    git_vector_init(&diff_list->conflicts, 0, NULL) < 0 ||
	    git_vector_init(&diff_list->resolved, 0, NULL) < 0) {
	    git_merge_diff_list__free(diff_list);
		return NULL;
	}

	return diff_list;
}


// Source: merge.c
// Lines 1834-1853
