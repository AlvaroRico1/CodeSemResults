void test_status_renames__head2index_no_rename_from_rewrite(void)
{
	git_index *index;
	git_status_list *statuslist;
	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	struct status_entry expected[] = {
		{ GIT_STATUS_INDEX_MODIFIED, "ikeepsix.txt", "ikeepsix.txt" },
		{ GIT_STATUS_INDEX_MODIFIED, "sixserving.txt", "sixserving.txt" },
	};

	opts.flags |= GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;

	cl_git_pass(git_repository_index(&index, g_repo));

	rename_file(g_repo, "ikeepsix.txt", "_temp_.txt");
	rename_file(g_repo, "sixserving.txt", "ikeepsix.txt");
	rename_file(g_repo, "_temp_.txt", "sixserving.txt");

	cl_git_pass(git_index_add_bypath(index, "ikeepsix.txt"));
	cl_git_pass(git_index_add_bypath(index, "sixserving.txt"));
	cl_git_pass(git_index_write(index));

	cl_git_pass(git_status_list_new(&statuslist, g_repo, &opts));
	check_status(statuslist, expected, 2);
	git_status_list_free(statuslist);

	git_index_free(index);
}


// Source: renames.c
// Lines 159-186
