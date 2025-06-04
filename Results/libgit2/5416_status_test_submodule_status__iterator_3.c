void test_submodule_status__iterator(void)
{
	git_iterator *iter;
	git_iterator_options iter_opts = GIT_ITERATOR_OPTIONS_INIT;
	const git_index_entry *entry;
	size_t i;
	static const char *expected[] = {
		".gitmodules",
		"just_a_dir/",
		"just_a_dir/contents",
		"just_a_file",
		"not-submodule/",
		"not-submodule/README.txt",
		"not/",
		"not/README.txt",
		"README.txt",
		"sm_added_and_uncommited",
		"sm_changed_file",
		"sm_changed_head",
		"sm_changed_index",
		"sm_changed_untracked_file",
		"sm_missing_commits",
		"sm_unchanged",
		NULL
	};
	static int expected_flags[] = {
		GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_WT_MODIFIED, /* ".gitmodules" */
		-1,					    /* "just_a_dir/" will be skipped */
		GIT_STATUS_CURRENT,     /* "just_a_dir/contents" */
		GIT_STATUS_CURRENT,	    /* "just_a_file" */
		GIT_STATUS_WT_NEW,      /* "not-submodule/" untracked item */
		-1,                     /* "not-submodule/README.txt" */
		GIT_STATUS_WT_NEW,      /* "not/" untracked item */
		-1,                     /* "not/README.txt" */
		GIT_STATUS_CURRENT,     /* "README.txt */
		GIT_STATUS_INDEX_NEW,   /* "sm_added_and_uncommited" */
		GIT_STATUS_WT_MODIFIED, /* "sm_changed_file" */
		GIT_STATUS_WT_MODIFIED, /* "sm_changed_head" */
		GIT_STATUS_WT_MODIFIED, /* "sm_changed_index" */
		GIT_STATUS_WT_MODIFIED, /* "sm_changed_untracked_file" */
		GIT_STATUS_WT_MODIFIED, /* "sm_missing_commits" */
		GIT_STATUS_CURRENT,     /* "sm_unchanged" */
		0
	};
	submodule_expectations exp = { 0, expected, expected_flags };
	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	git_index *index;

	iter_opts.flags = GIT_ITERATOR_IGNORE_CASE | GIT_ITERATOR_INCLUDE_TREES;

	cl_git_pass(git_repository_index(&index, g_repo));
	cl_git_pass(git_iterator_for_workdir(&iter, g_repo, index, NULL, &iter_opts));

	for (i = 0; !git_iterator_advance(&entry, iter); ++i)
		cl_assert_equal_s(expected[i], entry->path);

	git_iterator_free(iter);
	git_index_free(index);

	opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
		GIT_STATUS_OPT_INCLUDE_UNMODIFIED |
		GIT_STATUS_OPT_INCLUDE_IGNORED |
		GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
		GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;

	cl_git_pass(git_status_foreach_ext(
		g_repo, &opts, confirm_submodule_status, &exp));
}


// Source: status.c
// Lines 264-331
