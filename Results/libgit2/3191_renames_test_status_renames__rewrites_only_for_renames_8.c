void test_status_renames__rewrites_only_for_renames(void)
{
	git_index *index;
	git_status_list *statuslist;
	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	struct status_entry expected[] = {
		{ GIT_STATUS_WT_MODIFIED, "ikeepsix.txt", "ikeepsix.txt" },
	};

	opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;
	opts.flags |= GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;
	opts.flags |= GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR;
	opts.flags |= GIT_STATUS_OPT_RENAMES_FROM_REWRITES;

	cl_git_pass(git_repository_index(&index, g_repo));

	cl_git_rewritefile("renames/ikeepsix.txt",
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n" \
		"This is enough content for the file to be rewritten.\n");

	cl_git_pass(git_status_list_new(&statuslist, g_repo, &opts));
	check_status(statuslist, expected, 1);
	git_status_list_free(statuslist);

	git_index_free(index);
}


// Source: renames.c
// Lines 409-448
