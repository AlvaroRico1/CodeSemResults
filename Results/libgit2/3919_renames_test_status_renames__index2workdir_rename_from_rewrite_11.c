void test_status_renames__index2workdir_rename_from_rewrite(void)
{
	git_index *index;
	git_status_list *statuslist;
	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	struct status_entry expected[] = {
		{ GIT_STATUS_WT_RENAMED, "sixserving.txt", "ikeepsix.txt" },
		{ GIT_STATUS_WT_RENAMED, "ikeepsix.txt", "sixserving.txt" },
	};

	opts.flags |= GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR;
	opts.flags |= GIT_STATUS_OPT_RENAMES_FROM_REWRITES;

	cl_git_pass(git_repository_index(&index, g_repo));

	rename_file(g_repo, "ikeepsix.txt", "_temp_.txt");
	rename_file(g_repo, "sixserving.txt", "ikeepsix.txt");
	rename_file(g_repo, "_temp_.txt", "sixserving.txt");

	cl_git_pass(git_status_list_new(&statuslist, g_repo, &opts));
	check_status(statuslist, expected, 2);
	git_status_list_free(statuslist);

	git_index_free(index);
}


// Source: renames.c
// Lines 262-286
